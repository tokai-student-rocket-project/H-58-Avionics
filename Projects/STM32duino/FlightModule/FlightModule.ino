#include <TaskManager.h>
#include "CANSTM.hpp"
#include "PullupPin.hpp"
#include "OutputPin.hpp"
#include "DetectionCounter.hpp"
#include "Shiranui.hpp"
#include "Buzzer.hpp"
#include "AnalogVoltage.hpp"
#include "Logger.hpp"


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

  void changeMode(flightMode::Mode newMode);
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

namespace logger {
  Logger logger(D4);
}

namespace indicator {
  OutputPin canSend(D0);
  OutputPin canReceive(D1);

  Buzzer buzzer(A1, "buzzer");

  OutputPin flightModeBit0(D8);
  OutputPin flightModeBit1(D7);
  OutputPin flightModeBit2(D6);
  OutputPin flightModeBit3(D3);

  void indicateFlightMode(flightMode::Mode mode);
}

namespace control {
  DetectionCounter liftoffDetector(3);
  DetectionCounter resetDetector(10);

  OutputPin camera(D9);
  Shiranui sn3(A0, "sn3");
}

namespace connection {
  CANSTM can;
}

namespace data {
  float voltageSupply, voltageBattery, voltagePool;
  bool isFalling;
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
    while (!Serial);
    delay(800);
  }

  // デバッグ中はピンが干渉するので電圧監視を行わない
  if (!develop::isDebugMode) {
    analogReadResolution(12);
    sensor::supply.initialize(3300, 750);
    sensor::battery.initialize(4700, 820);
    sensor::pool.initialize(5600, 820);
  }

  SPI.setMOSI(A6);
  SPI.setMISO(A5);
  SPI.setSCLK(A4);
  SPI.begin();

  connection::can.begin();

  connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::SETUP);

  // 起動時はSLEEPモード
  flightMode::changeMode(flightMode::Mode::SLEEP);

  Tasks.add(timer::task10Hz)->startFps(10);
  Tasks.add(timer::task100Hz)->startFps(100);
}


void loop() {
  Tasks.update();

  // CAN受信処理
  if (connection::can.available()) {
    switch (connection::can.getLatestMessageLabel()) {
    case CANSTM::Label::TRAJECTORY_DATA:
      connection::can.receiveTrajectoryData(&data::isFalling);
      indicator::canReceive.toggle();
      break;
    }
  }
}


/// @brief X=0をセットする
void timer::setReferenceTime() {
  timer::referenceTime = millis();
}


/// @brief タイマーの時間を過ぎたかを返す
/// @param time 現在の時間
bool timer::isElapsedTime(uint32_t time) {
  return flightTime() >= time;
}


/// @brief 離昇してからの経過時間を返す
uint32_t timer::flightTime() {
  return millis() - timer::referenceTime;
}


/// @brief 10Hzで実行したい処理
void timer::task10Hz() {
  // その時のフライトモードに合わせてLEDを切り替える
  indicator::indicateFlightMode(flightMode::activeMode);


  // デバッグ中はピンが干渉するので電圧監視を行わない
  if (!develop::isDebugMode) {
    data::voltageSupply = sensor::supply.voltage();
    data::voltageBattery = sensor::battery.voltage();
    data::voltagePool = sensor::pool.voltage();
  }


  // CANにデータを流す
  connection::can.sendSystemStatus(
    static_cast<uint8_t>(flightMode::activeMode),
    control::camera.get(),
    control::sn3.get(),
    logger::logger.isLogging()
  );

  connection::can.sendScalar(CANSTM::Label::VOLTAGE_SUPPLY, data::voltageSupply);
  connection::can.sendScalar(CANSTM::Label::VOLTAGE_BATTERY, data::voltageBattery);
  connection::can.sendScalar(CANSTM::Label::VOLTAGE_POOL, data::voltagePool);
  indicator::canSend.toggle();
}


void timer::task100Hz() {
  // 検知の状態更新
  // フライトピンが解放されたらリフトオフ検知
  control::liftoffDetector.update(sensor::flightPin.isOpen());
  // フライトピンが閉鎖されたらリセット検知
  control::resetDetector.update(!sensor::flightPin.isOpen());


  // SLEEPモード以外の時にフライトピンが接続されたらリセット
  if (flightMode::activeMode != flightMode::Mode::SLEEP && control::resetDetector.isDetected()) {
    control::camera.off();
    indicator::buzzer.beepLongOnce();
    logger::logger.endLogging();
    flightMode::changeMode(flightMode::Mode::SLEEP);
    connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::RESET);
  }


  // 強制分離
  if (flightMode::activeMode == flightMode::Mode::CLIMB && isElapsedTime(timer::forceSeparation_time)) {
    control::sn3.separate();
    indicator::buzzer.beepTwice();
    flightMode::changeMode(flightMode::Mode::PARACHUTE);
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
      logger::logger.beginLogging();
      flightMode::changeMode(flightMode::Mode::STANDBY);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FLIGHT_MODE_ON);
    }

    // STANDBYモード 計測を開始し打ち上げを待つ
  case (flightMode::Mode::STANDBY):
    // フライトピン開放を検知すればTHRUSTモードに遷移する
    if (control::liftoffDetector.isDetected()) {
      // 現時刻をX=0の基準にする
      timer::setReferenceTime();
      indicator::buzzer.beepOnce();
      logger::logger.beginLogging();
      flightMode::changeMode(flightMode::Mode::THRUST);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::IGNITION);
    }
    break;

    // THRUSTモード 推進中
  case (flightMode::Mode::THRUST):
    // モータ作動時間を超えたら上昇モードに遷移
    if (timer::isElapsedTime(timer::thrust_time)) {
      flightMode::changeMode(flightMode::Mode::CLIMB);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::BURNOUT, flightTime());
    }
    break;

    // CLIMBモード 上昇中
  case (flightMode::Mode::CLIMB):
    // 頂点を検知すれば下降モードに遷移
    if (data::isFalling) {
      flightMode::changeMode(flightMode::Mode::DESCENT);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::APOGEE, flightTime());
    }
    break;

    // DESCENTモード 下降中
  case (flightMode::Mode::DESCENT):
    // 頂点分離なので分離保護時間を過ぎているならすぐに分離
    if (timer::isElapsedTime(timer::protectSeparation_time)) {
      control::sn3.separate();
      indicator::buzzer.beepTwice();
      flightMode::changeMode(flightMode::Mode::PARACHUTE);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::SEPARATE, flightTime());
    }
    break;

    // PARACHUTEモード パラシュート下降中
  case (flightMode::Mode::PARACHUTE):
    // 着地3秒前にカメラOFF
    if (control::camera.get() && timer::isElapsedTime(timer::land_time - 3000)) {
      control::camera.off();
    }

    // 着地時間を超えたら着地モードに遷移
    if (timer::isElapsedTime(timer::land_time)) {
      flightMode::changeMode(flightMode::Mode::LAND);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::LAND, flightTime());
    }
    break;

    // LANDモード 着地後も計測を行う
  case (flightMode::Mode::LAND):
    // シャットダウン時間を超えたらシャットダウン
    if (timer::isElapsedTime(timer::shutdown_time)) {
      indicator::buzzer.beepLongOnce();
      logger::logger.endLogging();
      flightMode::changeMode(flightMode::Mode::SHUTDOWN);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FLIGHT_MODE_OFF, flightTime());
      // はめつのうた
      // indicator::buzzer.electricalParade();
    }
    break;
  }


  // ログ保存
  if (logger::logger.isLogging()) {
    logger::logger.log(
      millis(), flightTime(),
      static_cast<uint8_t>(flightMode::activeMode), control::camera.get(), control::sn3.get(), logger::logger.isLogging(),
      data::isFalling, sensor::flightPin.isOpen(), !sensor::flightPin.isOpen(),
      data::voltageSupply, data::voltageBattery, data::voltagePool
    );
  }
}


/// @brief フライトモードを変更する
/// @param newMode 新しいモード
void flightMode::changeMode(flightMode::Mode newMode) {
  // フライトモードに変更がないなら何もしない
  if (flightMode::activeMode == newMode) {
    return;
  }

  flightMode::activeMode = newMode;
}


/// @brief フライトモードによって4つのLEDを切り替える
/// @param mode フライトモード
void indicator::indicateFlightMode(flightMode::Mode mode) {
  uint8_t modeNumber = static_cast<uint8_t>(mode);
  // 1つのLEDに1ビット分を当てはめる
  indicator::flightModeBit0.set(modeNumber & (1 << 0));
  indicator::flightModeBit1.set(modeNumber & (1 << 1));
  indicator::flightModeBit2.set(modeNumber & (1 << 2));
  indicator::flightModeBit3.set(modeNumber & (1 << 3));
}