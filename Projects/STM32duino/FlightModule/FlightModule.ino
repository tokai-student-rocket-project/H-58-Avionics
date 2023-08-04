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
#include "TimeManager.hpp"


namespace internal {
  void task4Hz();
  void task50Hz();
  void task100Hz();

  FlightModeManager flightModeManager;
  TimeManager timeManager;

  namespace flag {
    bool isFalling = false;
    bool isDebugMode = false;
    bool isLaunchMode = false;
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
    Switch bootSelect1(D12);
    Switch bootSelect2(D5);

    Switch flightPin(D11);

    DetectionCounter liftoffDetector(3);
    DetectionCounter resetDetector(10);
  }

  namespace peripheral {
    Logger logger(D4);
    Camera camera(D9);
    Shiranui sn3(D13, "sn3");
  }
}


namespace canbus {
  CANSTM can;
}

namespace data {
  float voltageSupply, voltageBattery, voltagePool;
}


void setup() {
  // デバッグ用シリアルポート
  // internal::flag::isDebugMode = true;
  // Serial.begin(115200);
  // while (!Serial);
  // delay(800);

  internal::timeManager.THRUST_TIME = 960;
  internal::timeManager.PROTECTION_SEPARATION_TIME = 5119;
  internal::timeManager.FORCE_SEPARATION_TIME = 6619;
  internal::timeManager.LANDING_TIME = 16700;
  internal::timeManager.SHUTDOWN_TIME = 21700;

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

  // CANで送り損ねないように若干ずらしたレートにする
  Tasks.add(internal::task4Hz)->startFps(5);
  Tasks.add(internal::task50Hz)->startFps(61);
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
    case CANSTM::Label::FLIGHT_MODE_ON_COMMAND:
      if (internal::flightModeManager.is(Var::FlightMode::SLEEP)) {
        internal::flightModeManager.changeMode(Var::FlightMode::STANDBY);
        device::peripheral::camera.on();
        device::peripheral::logger.beginLogging();
        device::indicator::buzzer.beepLongOnce();
        canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FLIGHT_MODE_ON);
      }

      device::indicator::canReceive.toggle();
      break;
    case CANSTM::Label::RESET_COMMAND:
      if (internal::flightModeManager.is(Var::FlightMode::STANDBY)) {
        internal::flightModeManager.changeMode(Var::FlightMode::SLEEP);
        device::peripheral::camera.off();
        device::peripheral::logger.endLogging();
        device::indicator::buzzer.beepLongOnce();
        canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::RESET);
      }

      if (internal::flightModeManager.isNot(Var::FlightMode::SLEEP) && internal::flightModeManager.isNot(Var::FlightMode::STANDBY)) {
        internal::flightModeManager.changeMode(Var::FlightMode::SHUTDOWN);
        device::peripheral::camera.off();
        device::peripheral::logger.endLogging();
        device::indicator::buzzer.beepLongOnce();
        canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FLIGHT_MODE_OFF, internal::timeManager.flightTime());
      }

      device::indicator::canReceive.toggle();
      break;
    case CANSTM::Label::VALVE_MODE: {
      bool isWaitingMode;
      canbus::can.receiveValveMode(&isWaitingMode);
      internal::flag::isLaunchMode = !isWaitingMode;
      break;
    }
    }
  }
}


/// @brief 4Hzで実行したい処理
void internal::task4Hz() {
  // デバッグ中はピンが干渉するので電圧監視を行わない
  if (!internal::flag::isDebugMode) {
    data::voltageSupply = device::sensor::supply.voltage();
    data::voltageBattery = device::sensor::battery.voltage();
    data::voltagePool = device::sensor::pool.voltage();
  }

  canbus::can.sendVoltage(data::voltageSupply, data::voltagePool, data::voltageBattery);
  device::indicator::canSend.toggle();
}


/// @brief 50Hzで実行したい処理
void internal::task50Hz() {
  // その時のフライトモードに合わせてLEDを切り替える
  device::indicator::indicateFlightMode(internal::flightModeManager.currentMode());


  // CANにデータを流す
  canbus::can.sendSystemStatus(
    internal::flightModeManager.currentMode(),
    static_cast<Var::State>(device::peripheral::camera.get()),
    static_cast<Var::State>(device::peripheral::sn3.get()),
    device::peripheral::logger.isLogging(),
    internal::flightModeManager.isFlying() ? internal::timeManager.flightTime() : -1
  );

  device::indicator::canSend.toggle();
}


void internal::task100Hz() {
  // 検知の状態更新
  // フライトピンが解放されたらリフトオフ検知
  device::detection::liftoffDetector.update(device::detection::flightPin.is(Var::SwitchState::OPEN));
  // フライトピンが閉鎖されたらリセット検知
  device::detection::resetDetector.update(device::detection::flightPin.is(Var::SwitchState::CLOSE));


  // SLEEPモードかSTANDBYモード以外の時にフライトピンが接続されたらリセット
  if (internal::flightModeManager.isNot(Var::FlightMode::SLEEP) && internal::flightModeManager.isNot(Var::FlightMode::STANDBY) && device::detection::resetDetector.isDetected()) {
    internal::flightModeManager.changeMode(Var::FlightMode::SLEEP);
    device::peripheral::camera.off();
    device::peripheral::logger.endLogging();
    device::indicator::buzzer.beepLongOnce();
    canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::RESET);
  }


  // 強制分離
  if ((internal::flightModeManager.is(Var::FlightMode::CLIMB) || internal::flightModeManager.is(Var::FlightMode::DESCENT)) && internal::timeManager.isElapsedTime(internal::timeManager.FORCE_SEPARATION_TIME)) {
    internal::flightModeManager.changeMode(Var::FlightMode::PARACHUTE);
    device::peripheral::sn3.separate();
    device::indicator::buzzer.beepTwice();
    canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FORCE_SEPARATE, internal::timeManager.flightTime());
  }


  // 条件が揃えばフライトモードを遷移する
  switch (internal::flightModeManager.currentMode()) {
  case (Var::FlightMode::SLEEP): {
    // SLEEPモード 打ち上げを静かに待つ

    // バルブ開信号かフライトモードオンコマンドを受信すればスタンバイモードに遷移する
    // フライトピン開放 || バルブ開
    if (device::detection::liftoffDetector.isDetected() || internal::flag::isLaunchMode) {
      internal::flightModeManager.changeMode(Var::FlightMode::STANDBY);
      device::peripheral::camera.on();
      device::peripheral::logger.beginLogging();
      device::indicator::buzzer.beepLongOnce();
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FLIGHT_MODE_ON);
    }

    break;
  }

  case (Var::FlightMode::STANDBY): {
    // STANDBYモード 計測を開始し打ち上げを待つ

    // フライトピン開放を検知すればTHRUSTモードに遷移する
    if (device::detection::liftoffDetector.isDetected()) {
      internal::flightModeManager.changeMode(Var::FlightMode::THRUST);
      // 現時刻をX=0の基準にする
      internal::timeManager.setZero();
      device::peripheral::logger.beginLogging();
      device::indicator::buzzer.beepOnce();
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::IGNITION);
    }

    break;
  }

  case (Var::FlightMode::THRUST): {
    // THRUSTモード 推進中

    // モータ作動時間を超えたら上昇モードに遷移
    if (internal::timeManager.isElapsedTime(internal::timeManager.THRUST_TIME)) {
      internal::flightModeManager.changeMode(Var::FlightMode::CLIMB);
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::BURNOUT, internal::timeManager.flightTime());
    }

    break;
  }

  case (Var::FlightMode::CLIMB): {
    // CLIMBモード 上昇中

    // 頂点を検知すれば下降モードに遷移
    if (internal::flag::isFalling) {
      internal::flightModeManager.changeMode(Var::FlightMode::DESCENT);
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::APOGEE, internal::timeManager.flightTime());
    }

    break;
  }

  case (Var::FlightMode::DESCENT): {
    // DESCENTモード 下降中

    // 頂点分離なので分離保護時間を過ぎているならすぐに分離
    if (internal::timeManager.isElapsedTime(internal::timeManager.PROTECTION_SEPARATION_TIME)) {
      internal::flightModeManager.changeMode(Var::FlightMode::PARACHUTE);
      device::peripheral::sn3.separate();
      device::indicator::buzzer.beepTwice();
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::SEPARATE, internal::timeManager.flightTime());
    }

    break;
  }

  case (Var::FlightMode::PARACHUTE): {
    // PARACHUTEモード パラシュート下降中

    // 着地3秒前にカメラOFF
    if (device::peripheral::camera.get() == Var::State::ON && internal::timeManager.isElapsedTime(internal::timeManager.LANDING_TIME - 3000)) {
      device::peripheral::camera.off();
    }

    // 着地時間を超えたら着地モードに遷移
    if (internal::timeManager.isElapsedTime(internal::timeManager.LANDING_TIME)) {
      internal::flightModeManager.changeMode(Var::FlightMode::LAND);
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::LAND, internal::timeManager.flightTime());
    }

    break;
  }

  case (Var::FlightMode::LAND): {
    // LANDモード 着地後も計測を行う

    // シャットダウン時間を超えたらシャットダウン
    if (internal::timeManager.isElapsedTime(internal::timeManager.SHUTDOWN_TIME)) {
      internal::flightModeManager.changeMode(Var::FlightMode::SHUTDOWN);
      device::peripheral::logger.endLogging();
      device::indicator::buzzer.beepLongOnce();
      canbus::can.sendEvent(CANSTM::Publisher::FLIGHT_MODULE, CANSTM::EventCode::FLIGHT_MODE_OFF, internal::timeManager.flightTime());
      // はめつのうた
      // indicator::buzzer.electricalParade();
    }

    break;
  }
  }


  // ログ保存
  if (device::peripheral::logger.isLogging()) {
    device::peripheral::logger.log(
      millis(), internal::timeManager.flightTime(),
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