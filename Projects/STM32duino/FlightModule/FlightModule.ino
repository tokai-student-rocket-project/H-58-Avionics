#include <TaskManager.h>
#include "CANSTM.hpp"
#include "Switch.hpp"
#include "LED.hpp"
#include "DetectionCounter.hpp"
#include "Shiranui.hpp"
#include "Buzzer.hpp"
#include "Camera.hpp"
#include "AnalogVoltage.hpp"
#include "Logger.hpp"
#include "Var.hpp"
#include "FlightModeManager.hpp"


// TODO namespace 整理
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

  Switch flightPin(D11);
  DetectionCounter liftoffDetector(3);
  DetectionCounter resetDetector(10);
}

namespace logger {
  Logger logger(D4);
}

namespace indicator {
  LED canSend(D0);
  LED canReceive(D1);

  Buzzer buzzer(A1, "buzzer");

  LED flightModeBit0(D8);
  LED flightModeBit1(D7);
  LED flightModeBit2(D6);
  LED flightModeBit3(D3);

  void indicateFlightMode(Var::FlightMode mode);
}

namespace control {
  DetectionCounter liftoffDetector(3);
  DetectionCounter resetDetector(10);

  Camera camera(D9);
  Shiranui sn3(A0, "sn3");

  FlightModeManager flightModeManager;
}

namespace connection {
  CANSTM can;
}

namespace data {
  float voltageSupply, voltageBattery, voltagePool;
  bool isFalling;
}

namespace develop {
  Switch debugMode(D12);

  bool isDebugMode;
}


void setup() {
  // TODO なくす
  // 起動モードの判定
  develop::isDebugMode = develop::debugMode.is(Var::SwitchState::CLOSE);

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
  indicator::indicateFlightMode(control::flightModeManager.currentMode());


  // デバッグ中はピンが干渉するので電圧監視を行わない
  if (!develop::isDebugMode) {
    data::voltageSupply = sensor::supply.voltage();
    data::voltageBattery = sensor::battery.voltage();
    data::voltagePool = sensor::pool.voltage();
  }


  // CANにデータを流す
  connection::can.sendSystemStatus(
    control::flightModeManager.currentMode(),
    static_cast<Var::State>(control::camera.get()),
    static_cast<Var::State>(control::sn3.get()),
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
  control::liftoffDetector.update(sensor::flightPin.is(Var::SwitchState::OPEN));
  // フライトピンが閉鎖されたらリセット検知
  control::resetDetector.update(sensor::flightPin.is(Var::SwitchState::CLOSE));


  // SLEEPモード以外の時にフライトピンが接続されたらリセット
  if (control::flightModeManager.isNot(Var::FlightMode::SLEEP) && control::resetDetector.isDetected()) {
    control::camera.off();
    indicator::buzzer.beepLongOnce();
    logger::logger.endLogging();
    control::flightModeManager.changeMode(Var::FlightMode::SLEEP);
    connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::RESET);
  }


  // 強制分離
  if (control::flightModeManager.is(Var::FlightMode::CLIMB) && isElapsedTime(timer::forceSeparation_time)) {
    control::sn3.separate();
    indicator::buzzer.beepTwice();
    control::flightModeManager.changeMode(Var::FlightMode::PARACHUTE);
    connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FORCE_SEPARATE, flightTime());
  }


  // 条件が揃えばフライトモードを遷移する
  switch (control::flightModeManager.currentMode()) {
  case (Var::FlightMode::SLEEP): {
    // SLEEPモード 打ち上げを静かに待つ

    // バルブ開信号かフライトモードオンコマンドを受信すればスタンバイモードに遷移する
    // フライトピン開放 || バルブ開 || FlightMode ON
    if (control::liftoffDetector.isDetected() || false || false) {
      control::camera.on();
      logger::logger.beginLogging();
      control::flightModeManager.changeMode(Var::FlightMode::STANDBY);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FLIGHT_MODE_ON);
    }

    break;
  }

  case (Var::FlightMode::STANDBY): {
    // STANDBYモード 計測を開始し打ち上げを待つ

    // フライトピン開放を検知すればTHRUSTモードに遷移する
    if (control::liftoffDetector.isDetected()) {
      // 現時刻をX=0の基準にする
      timer::setReferenceTime();
      indicator::buzzer.beepOnce();
      logger::logger.beginLogging();
      control::flightModeManager.changeMode(Var::FlightMode::THRUST);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::IGNITION);
    }

    break;
  }

  case (Var::FlightMode::THRUST): {
    // THRUSTモード 推進中

    // モータ作動時間を超えたら上昇モードに遷移
    if (timer::isElapsedTime(timer::thrust_time)) {
      control::flightModeManager.changeMode(Var::FlightMode::CLIMB);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::BURNOUT, flightTime());
    }

    break;
  }

  case (Var::FlightMode::CLIMB): {
    // CLIMBモード 上昇中

    // 頂点を検知すれば下降モードに遷移
    if (data::isFalling) {
      control::flightModeManager.changeMode(Var::FlightMode::DESCENT);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::APOGEE, flightTime());
    }

    break;
  }

  case (Var::FlightMode::DESCENT): {
    // DESCENTモード 下降中

    // 頂点分離なので分離保護時間を過ぎているならすぐに分離
    if (timer::isElapsedTime(timer::protectSeparation_time)) {
      control::sn3.separate();
      indicator::buzzer.beepTwice();
      control::flightModeManager.changeMode(Var::FlightMode::PARACHUTE);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::SEPARATE, flightTime());
    }

    break;
  }

  case (Var::FlightMode::PARACHUTE): {
    // PARACHUTEモード パラシュート下降中

    // 着地3秒前にカメラOFF
    if (control::camera.get() == Var::State::ON && timer::isElapsedTime(timer::land_time - 3000)) {
      control::camera.off();
    }

    // 着地時間を超えたら着地モードに遷移
    if (timer::isElapsedTime(timer::land_time)) {
      control::flightModeManager.changeMode(Var::FlightMode::LAND);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::LAND, flightTime());
    }

    break;
  }

  case (Var::FlightMode::LAND): {
    // LANDモード 着地後も計測を行う

    // シャットダウン時間を超えたらシャットダウン
    if (timer::isElapsedTime(timer::shutdown_time)) {
      indicator::buzzer.beepLongOnce();
      logger::logger.endLogging();
      control::flightModeManager.changeMode(Var::FlightMode::SHUTDOWN);
      connection::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FLIGHT_MODE_OFF, flightTime());
      // はめつのうた
      // indicator::buzzer.electricalParade();
    }

    break;
  }
  }


  // ログ保存
  if (logger::logger.isLogging()) {
    logger::logger.log(
      millis(), flightTime(),
      static_cast<uint8_t>(control::flightModeManager.currentMode()),
      static_cast<uint8_t>(control::camera.get()),
      static_cast<uint8_t>(control::sn3.get()),
      logger::logger.isLogging(),
      data::isFalling,
      static_cast<uint8_t>(sensor::flightPin.is(Var::SwitchState::CLOSE)),
      static_cast<uint8_t>(sensor::flightPin.is(Var::SwitchState::OPEN)),
      data::voltageSupply, data::voltageBattery, data::voltagePool
    );
  }
}


/// @brief フライトモードによって4つのLEDを切り替える
/// @param mode フライトモード
void indicator::indicateFlightMode(Var::FlightMode mode) {
  uint8_t modeNumber = static_cast<uint8_t>(mode);
  // 1つのLEDに1ビット分を当てはめる
  indicator::flightModeBit0.set(modeNumber & (1 << 0) ? Var::State::ON : Var::State::OFF);
  indicator::flightModeBit1.set(modeNumber & (1 << 1) ? Var::State::ON : Var::State::OFF);
  indicator::flightModeBit2.set(modeNumber & (1 << 2) ? Var::State::ON : Var::State::OFF);
  indicator::flightModeBit3.set(modeNumber & (1 << 3) ? Var::State::ON : Var::State::OFF);
}