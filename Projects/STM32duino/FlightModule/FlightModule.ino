#include <TaskManager.h>
#include "CANSTM.hpp"
#include "PullupPin.hpp"
#include "OutputPin.hpp"
#include "DetectionCounter.hpp"
#include "ApogeeDetector.hpp"
#include "Shiranui.hpp"
#include "AnalogVoltage.hpp"


namespace flightMode {
  enum class Mode : uint8_t {
    SLEEP,
    STANDBY,
    THRUST,
    CLIMB,
    DESCENT,
    DECEL,
    PARACHUTE,
    LAND,
    SHUTDOWN
  };

  Mode activeMode;
}

namespace timer {
  uint32_t thrust_time = 3000;
  uint32_t protectSeparation_time = 7000;
  uint32_t forceSeparation_time = 10000;
  uint32_t land_time = 25000;
  uint32_t shutdown_time = 26000;

  uint32_t referenceTime;

  void setReferenceTime();
  bool isElapsedTime(uint32_t time);
  uint32_t flightTime();

  void task10Hz();
  void task100Hz();
}

namespace sensor {
  AnalogVoltage supply(A7);
  AnalogVoltage battery(A2);
  AnalogVoltage pool(A3);

  PullupPin flightPin(D11);
  DetectionCounter liftoffDetector(3);
  DetectionCounter resetDetector(10);
}

namespace indicator {
  OutputPin canSend(D0);
  OutputPin canReceive(D1);

  OutputPin flightModeBit0(D8);
  OutputPin flightModeBit1(D7);
  OutputPin flightModeBit2(D6);
  OutputPin flightModeBit3(D3);

  void indicateFlightMode(flightMode::Mode mode);
}

namespace control {
  DetectionCounter liftoffDetector(3);
  DetectionCounter resetDetector(10);
  ApogeeDetector apogeeDetector(0.25, 0.75);

  OutputPin camera(D9);
  Shiranui sn3(A0, "sn3");
}

namespace connection {
  CANSTM can;
}

namespace data {
  float altitude;

  float voltageSupply, voltageBattery, voltagePool;
}

namespace develop {
  PullupPin debugMode(D12);

  bool isDebugMode;
}


void setup() {
  // 起動モードの判定
  develop::isDebugMode = !develop::debugMode.isOpen();

  // デバッグ用シリアルポートの準備
  if (develop::isDebugMode) {
    Serial.begin(115200);
    delay(800);
  }

  // デバッグ中はピンが干渉するので電圧監視を行わない
  if (!develop::isDebugMode) {
    analogReadResolution(12);
    sensor::supply.begin(3300, 750);
    sensor::battery.begin(4700, 820);
    sensor::pool.begin(5600, 820);
  }

  connection::can.begin();

  connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::SETUP);
  flightMode::activeMode = flightMode::Mode::SLEEP;

  Tasks.add(timer::task10Hz)->startFps(10);
  Tasks.add(timer::task100Hz)->startFps(100);
}


void loop() {
  Tasks.update();

  // CAN受信処理
  if (connection::can.available()) {
    switch (connection::can.getLatestMessageLabel()) {
    case CANSTM::Label::ALTITUDE:
      connection::can.receiveScalar(&data::altitude);
      break;
    }

    indicator::canReceive.toggle();
  }
}


/// @brief X=0をセットする
void timer::setReferenceTime() {
  timer::referenceTime = millis();
}


/// @brief タイマーの時間を過ぎたか
/// @param time 現在の時間
bool timer::isElapsedTime(uint32_t time) {
  return flightTime() >= time;
}


/// @brief 離昇してからの経過時間を返す
uint32_t timer::flightTime() {
  return millis() - timer::referenceTime;
}


void timer::task10Hz() {
  // その時のフライトモードに合わせてLEDを切り替える
  indicator::indicateFlightMode(flightMode::activeMode);


  // 検知の状態更新
  control::liftoffDetector.update(sensor::flightPin.isOpen());
  control::resetDetector.update(!sensor::flightPin.isOpen());
  control::apogeeDetector.update(data::altitude);


  // デバッグ中はピンが干渉するので電圧監視を行わない
  if (!develop::isDebugMode) {
    data::voltageSupply = sensor::supply.voltage();
    data::voltageBattery = sensor::battery.voltage();
    data::voltagePool = sensor::pool.voltage();
  }


  connection::can.sendSystemStatus(
    static_cast<uint8_t>(flightMode::activeMode),
    control::camera.get(),
    control::sn3.get()
  );

  connection::can.sendScalar(CANSTM::Label::VOLTAGE_SUPPLY, data::voltageSupply);
  connection::can.sendScalar(CANSTM::Label::VOLTAGE_BATTERY, data::voltageBattery);
  connection::can.sendScalar(CANSTM::Label::VOLTAGE_POOL, data::voltagePool);
  indicator::canSend.toggle();
}


void timer::task100Hz() {
  // SLEEPモード以外の時にフライトピンが接続されたらリセット
  if (flightMode::activeMode != flightMode::Mode::SLEEP && control::resetDetector.isDetected()) {
    control::camera.off();
    flightMode::activeMode = flightMode::Mode::SLEEP;
    connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::RESET);
  }


  // 強制分離
  if (flightMode::activeMode == flightMode::Mode::CLIMB && isElapsedTime(timer::forceSeparation_time)) {
    control::sn3.separate();
    flightMode::activeMode = flightMode::Mode::PARACHUTE;
    connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FORCE_SEPARATE, flightTime());
  }


  // 条件が揃えばフライトモードを遷移する
  switch (flightMode::activeMode) {
    // SLEEPモード 打ち上げを静かに待つ
  case (flightMode::Mode::SLEEP):
    // バルブ開信号かフライトモードオンコマンドを受信すればスタンバイモードに遷移する
    // フライトピン開放 || バルブ開 || FlightMode ON
    if (control::liftoffDetector.isDetected() || false || false) {
      control::camera.on();
      flightMode::activeMode = flightMode::Mode::STANDBY;
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FLIGHT_MODE_ON);
    }

    // STANDBYモード 計測を開始し打ち上げを待つ
  case (flightMode::Mode::STANDBY):
    // フライトピン開放を検知すればTHRUSTモードに遷移する
    if (control::liftoffDetector.isDetected()) {
      // 現時刻をX=0の基準にする
      timer::setReferenceTime();
      flightMode::activeMode = flightMode::Mode::THRUST;
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::IGNITION);
    }
    break;

    // THRUSTモード 推進中
  case (flightMode::Mode::THRUST):
    // モータ作動時間を超えたら上昇モードに遷移
    if (timer::isElapsedTime(timer::thrust_time)) {
      flightMode::activeMode = flightMode::Mode::CLIMB;
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::BURNOUT, flightTime());
    }
    break;

    // CLIMBモード 上昇中
  case (flightMode::Mode::CLIMB):
    // 頂点を検知すれば下降モードに遷移
    if (control::apogeeDetector.isDetected()) {
      flightMode::activeMode = flightMode::Mode::DESCENT;
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::APOGEE, flightTime());
    }
    break;

    // DESCENTモード 下降中
  case (flightMode::Mode::DESCENT):
    // 頂点分離なので分離保護時間を過ぎているならすぐに分離
    if (timer::isElapsedTime(timer::protectSeparation_time)) {
      control::sn3.separate();
      flightMode::activeMode = flightMode::Mode::PARACHUTE;
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::SEPARATE, flightTime());
    }
    break;

    // PARACHUTEモード パラシュート下降中
  case (flightMode::Mode::PARACHUTE):
    // 着地時間を超えたら着地モードに遷移
    if (timer::isElapsedTime(timer::land_time)) {
      flightMode::activeMode = flightMode::Mode::LAND;
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::LAND, flightTime());
    }
    break;

    // LANDモード 着地後も計測を行う
  case (flightMode::Mode::LAND):
    // シャットダウン時間を超えたらシャットダウン
    if (timer::isElapsedTime(timer::shutdown_time)) {
      control::camera.off();
      flightMode::activeMode = flightMode::Mode::SHUTDOWN;
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FLIGHT_MODE_OFF, flightTime());
    }
    break;
  }
}


void indicator::indicateFlightMode(flightMode::Mode mode) {
  uint8_t modeNumber = static_cast<uint8_t>(mode);
  indicator::flightModeBit0.set(modeNumber & (1 << 0));
  indicator::flightModeBit1.set(modeNumber & (1 << 1));
  indicator::flightModeBit2.set(modeNumber & (1 << 2));
  indicator::flightModeBit3.set(modeNumber & (1 << 3));
}