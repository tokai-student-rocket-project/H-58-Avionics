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


namespace internal {
  void task10Hz();
  void task100Hz();

  FlightModeManager flightModeManager;

  namespace flag {
    bool isFalling;
    bool isDebugMode;
  }

  namespace time {
    uint32_t thrust_time = 3000;
    uint32_t protectSeparation_time = 7000;
    uint32_t forceSeparation_time = 10000;
    uint32_t land_time = 25000;
    uint32_t shutdown_time = 26000;

    uint32_t referenceTime;

    void setReferenceTime();
    bool isElapsedTime(uint32_t time);
    uint32_t flightTime();
  }
}

namespace device {
  namespace sensor {
    AnalogVoltage supply(A7);
    AnalogVoltage battery(A2);
    AnalogVoltage pool(A3);
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

  namespace detection {
    Switch flightPin(D11);
    Switch debugMode(D12);

    DetectionCounter liftoffDetector(3);
    DetectionCounter resetDetector(10);
  }

  namespace peripheral {
    Logger logger(D4);
    Camera camera(D9);
    Shiranui sn3(A0, "sn3");
  }
}


namespace canbus {
  CANSTM can;
}

namespace data {
  float voltageSupply, voltageBattery, voltagePool;
}


void setup() {
  // TODO なくす
  // 起動モードの判定
  internal::flag::isDebugMode = device::detection::debugMode.is(Var::SwitchState::CLOSE);

  // デバッグ用シリアルポートの準備
  if (internal::flag::isDebugMode) {
    Serial.begin(115200);
    while (!Serial);
    delay(800);
  }

  // デバッグ中はピンが干渉するので電圧監視を行わない
  if (!internal::flag::isDebugMode) {
    analogReadResolution(12);
    device::sensor::supply.initialize(3300, 750);
    device::sensor::battery.initialize(4700, 820);
    device::sensor::pool.initialize(5600, 820);
  }

  SPI.setMOSI(A6);
  SPI.setMISO(A5);
  SPI.setSCLK(A4);
  SPI.begin();

  canbus::can.begin();

  canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::SETUP);

  Tasks.add(internal::task10Hz)->startFps(10);
  Tasks.add(internal::task100Hz)->startFps(100);
}


void loop() {
  Tasks.update();

  // CAN受信処理
  if (canbus::can.available()) {
    switch (canbus::can.getLatestMessageLabel()) {
    case CANSTM::Label::TRAJECTORY_DATA:
      canbus::can.receiveTrajectoryData(&internal::flag::isFalling);
      device::indicator::canReceive.toggle();
      break;
    }
  }
}


/// @brief X=0をセットする
void internal::time::setReferenceTime() {
  internal::time::referenceTime = millis();
}


/// @brief タイマーの時間を過ぎたかを返す
/// @param time 現在の時間
bool internal::time::isElapsedTime(uint32_t time) {
  return internal::time::flightTime() >= time;
}


/// @brief 離昇してからの経過時間を返す
uint32_t internal::time::flightTime() {
  return millis() - internal::time::referenceTime;
}


/// @brief 10Hzで実行したい処理
void internal::task10Hz() {
  // その時のフライトモードに合わせてLEDを切り替える
  device::indicator::indicateFlightMode(internal::flightModeManager.currentMode());


  // デバッグ中はピンが干渉するので電圧監視を行わない
  if (!internal::flag::isDebugMode) {
    data::voltageSupply = device::sensor::supply.voltage();
    data::voltageBattery = device::sensor::battery.voltage();
    data::voltagePool = device::sensor::pool.voltage();
  }


  // CANにデータを流す
  canbus::can.sendSystemStatus(
    internal::flightModeManager.currentMode(),
    static_cast<Var::State>(device::peripheral::camera.get()),
    static_cast<Var::State>(device::peripheral::sn3.get()),
    device::peripheral::logger.isLogging()
  );

  canbus::can.sendScalar(CANSTM::Label::VOLTAGE_SUPPLY, data::voltageSupply);
  canbus::can.sendScalar(CANSTM::Label::VOLTAGE_BATTERY, data::voltageBattery);
  canbus::can.sendScalar(CANSTM::Label::VOLTAGE_POOL, data::voltagePool);
  device::indicator::canSend.toggle();
}


void internal::task100Hz() {
  // 検知の状態更新
  // フライトピンが解放されたらリフトオフ検知
  device::detection::liftoffDetector.update(device::detection::flightPin.is(Var::SwitchState::OPEN));
  // フライトピンが閉鎖されたらリセット検知
  device::detection::resetDetector.update(device::detection::flightPin.is(Var::SwitchState::CLOSE));


  // SLEEPモード以外の時にフライトピンが接続されたらリセット
  if (internal::flightModeManager.isNot(Var::FlightMode::SLEEP) && device::detection::resetDetector.isDetected()) {
    device::peripheral::camera.off();
    device::indicator::buzzer.beepLongOnce();
    device::peripheral::logger.endLogging();
    internal::flightModeManager.changeMode(Var::FlightMode::SLEEP);
    canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::RESET);
  }


  // 強制分離
  if (internal::flightModeManager.is(Var::FlightMode::CLIMB) && internal::time::isElapsedTime(internal::time::forceSeparation_time)) {
    device::peripheral::sn3.separate();
    device::indicator::buzzer.beepTwice();
    internal::flightModeManager.changeMode(Var::FlightMode::PARACHUTE);
    canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FORCE_SEPARATE, internal::time::flightTime());
  }


  // 条件が揃えばフライトモードを遷移する
  switch (internal::flightModeManager.currentMode()) {
  case (Var::FlightMode::SLEEP): {
    // SLEEPモード 打ち上げを静かに待つ

    // バルブ開信号かフライトモードオンコマンドを受信すればスタンバイモードに遷移する
    // フライトピン開放 || バルブ開 || FlightMode ON
    if (device::detection::liftoffDetector.isDetected() || false || false) {
      device::peripheral::camera.on();
      device::peripheral::logger.beginLogging();
      internal::flightModeManager.changeMode(Var::FlightMode::STANDBY);
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FLIGHT_MODE_ON);
    }

    break;
  }

  case (Var::FlightMode::STANDBY): {
    // STANDBYモード 計測を開始し打ち上げを待つ

    // フライトピン開放を検知すればTHRUSTモードに遷移する
    if (device::detection::liftoffDetector.isDetected()) {
      // 現時刻をX=0の基準にする
      internal::time::setReferenceTime();
      device::indicator::buzzer.beepOnce();
      device::peripheral::logger.beginLogging();
      internal::flightModeManager.changeMode(Var::FlightMode::THRUST);
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::IGNITION);
    }

    break;
  }

  case (Var::FlightMode::THRUST): {
    // THRUSTモード 推進中

    // モータ作動時間を超えたら上昇モードに遷移
    if (internal::time::isElapsedTime(internal::time::thrust_time)) {
      internal::flightModeManager.changeMode(Var::FlightMode::CLIMB);
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::BURNOUT, internal::time::flightTime());
    }

    break;
  }

  case (Var::FlightMode::CLIMB): {
    // CLIMBモード 上昇中

    // 頂点を検知すれば下降モードに遷移
    if (internal::flag::isFalling) {
      internal::flightModeManager.changeMode(Var::FlightMode::DESCENT);
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::APOGEE, internal::time::flightTime());
    }

    break;
  }

  case (Var::FlightMode::DESCENT): {
    // DESCENTモード 下降中

    // 頂点分離なので分離保護時間を過ぎているならすぐに分離
    if (internal::time::isElapsedTime(internal::time::protectSeparation_time)) {
      device::peripheral::sn3.separate();
      device::indicator::buzzer.beepTwice();
      internal::flightModeManager.changeMode(Var::FlightMode::PARACHUTE);
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::SEPARATE, internal::time::flightTime());
    }

    break;
  }

  case (Var::FlightMode::PARACHUTE): {
    // PARACHUTEモード パラシュート下降中

    // 着地3秒前にカメラOFF
    if (device::peripheral::camera.get() == Var::State::ON && internal::time::isElapsedTime(internal::time::land_time - 3000)) {
      device::peripheral::camera.off();
    }

    // 着地時間を超えたら着地モードに遷移
    if (internal::time::isElapsedTime(internal::time::land_time)) {
      internal::flightModeManager.changeMode(Var::FlightMode::LAND);
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::LAND, internal::time::flightTime());
    }

    break;
  }

  case (Var::FlightMode::LAND): {
    // LANDモード 着地後も計測を行う

    // シャットダウン時間を超えたらシャットダウン
    if (internal::time::isElapsedTime(internal::time::shutdown_time)) {
      device::indicator::buzzer.beepLongOnce();
      device::peripheral::logger.endLogging();
      internal::flightModeManager.changeMode(Var::FlightMode::SHUTDOWN);
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FLIGHT_MODE_OFF, internal::time::flightTime());
      // はめつのうた
      // indicator::buzzer.electricalParade();
    }

    break;
  }
  }


  // ログ保存
  if (device::peripheral::logger.isLogging()) {
    device::peripheral::logger.log(
      millis(), internal::time::flightTime(),
      static_cast<uint8_t>(internal::flightModeManager.currentMode()),
      static_cast<uint8_t>(device::peripheral::camera.get()),
      static_cast<uint8_t>(device::peripheral::sn3.get()),
      device::peripheral::logger.isLogging(),
      internal::flag::isFalling,
      static_cast<uint8_t>(device::detection::flightPin.is(Var::SwitchState::CLOSE)),
      static_cast<uint8_t>(device::detection::flightPin.is(Var::SwitchState::OPEN)),
      data::voltageSupply, data::voltageBattery, data::voltagePool
    );
  }
}


/// @brief フライトモードによって4つのLEDを切り替える
/// @param mode フライトモード
void device::indicator::indicateFlightMode(Var::FlightMode mode) {
  uint8_t modeNumber = static_cast<uint8_t>(mode);
  // 1つのLEDに1ビット分を当てはめる
  device::indicator::flightModeBit0.set(modeNumber & (1 << 0) ? Var::State::ON : Var::State::OFF);
  device::indicator::flightModeBit1.set(modeNumber & (1 << 1) ? Var::State::ON : Var::State::OFF);
  device::indicator::flightModeBit2.set(modeNumber & (1 << 2) ? Var::State::ON : Var::State::OFF);
  device::indicator::flightModeBit3.set(modeNumber & (1 << 3) ? Var::State::ON : Var::State::OFF);
}