#include <Wire.h>
#include <TaskManager.h>
#include "CANSTM.hpp"
#include "BNO055.hpp"
#include "BME280.hpp"
#include "Thermistor.hpp"
#include "Switch.hpp"
#include "LED.hpp"
#include "Trajectory.hpp"
#include "Logger.hpp"
#include "Var.hpp"


namespace internal {
  void task02Hz();
  void task2Hz();
  void task20Hz();
  void task50Hz();
  void task100Hz();

  Trajectory trajectory;
}

namespace device {
  namespace sensor {
    BNO055 bno;
    BME bme;
    Thermistor thermistor(A1);
  }

  namespace indicator {
    LED canSend(D12);
    LED canReceive(D11);
    LED sdStatus(D9);
    LED loggerStatus(D6);
  }

  namespace detection {
    Switch bootSelect1(D3);
    Switch bootSelect2(A7);

    Switch cardDetection(D8);
  }

  namespace peripheral {
    Logger logger(A2, A3, A0);
    LED recorderPower(D7);
  }
}

namespace canbus {
  CANSTM can;

  void handleSystemStatus();
  void handleSetReferencePressure();
}

namespace data {
  float pressure_hPa;
  float outsideTemperature_degC, internalTemperature_degC;
  float altitude_m;
  float climbRate_mps;
  float acceleration_x_mps2, acceleration_y_mps2, acceleration_z_mps2;
  float magnetometer_x_nT, magnetometer_y_nT, magnetometer_z_nT;
  float gyroscope_x_dps, gyroscope_y_dps, gyroscope_z_dps;
  float orientation_x_deg, orientation_y_deg, orientation_z_deg;
  float linear_acceleration_x_mps2, linear_acceleration_y_mps2, linear_acceleration_z_mps2;
  float gravity_x_mps2, gravity_y_mps2, gravity_z_mps2;
  float quaternion_w, quaternion_x, quaternion_y, quaternion_z;
  float collected_temperature, cold_junction_temperature, thermo_couple_temperature;

  Var::FlightMode flightMode;
}


void setup() {
  // デバッグ用シリアルポート
  // Serial.begin(115200);
  // while (!Serial);
  // delay(800);

  // FRAMとSDの電源は常にON
  device::peripheral::recorderPower.on();

  SPI.setMOSI(A6);
  SPI.setMISO(A5);
  SPI.setSCLK(A4);
  SPI.begin();

  Wire.setSDA(D4);
  Wire.setSCL(D5);
  Wire.begin();
  Wire.setClock(400000);

  device::sensor::bno.begin();
  device::sensor::bme.begin();
  device::sensor::thermistor.initialize();

  canbus::can.begin();
  canbus::can.sendEvent(CANSTM::Publisher::SENSING_MODULE, CANSTM::EventCode::SETUP);

  // CANで送り損ねないように若干ずらしたレートにする
  Tasks.add(internal::task02Hz)->startIntervalSec(5);
  Tasks.add(internal::task2Hz)->startFps(2);
  Tasks.add(internal::task20Hz)->startFps(29);
  Tasks.add(internal::task50Hz)->startFps(53);
  Tasks.add(internal::task100Hz)->startFps(100);
}


void loop() {
  Tasks.update();

  //CAN受信処理
  if (canbus::can.available()) {
    switch (canbus::can.getLatestMessageLabel()) {
    case CANSTM::Label::SYSTEM_STATUS:
      canbus::handleSystemStatus();
      device::indicator::canReceive.toggle();
      break;
    case CANSTM::Label::SET_REFERENCE_PRESSURE_COMMAND:
      canbus::handleSetReferencePressure();
      device::indicator::canReceive.toggle();
      break;
    case CANSTM::Label::COLLECTED_TEMPERATURE:
      canbus::can.receiveScalaDouble(&data::collected_temperature);
      break;
    case CANSTM::Label::COLD_JUNCTION_TEMPERATURE:
      canbus::can.receiveScalaDouble(&data::cold_junction_temperature);
      break;
    case CANSTM::Label::THERMO_COUPLE_TEMPERATURE:
      canbus::can.receiveScalaDouble(&data::thermo_couple_temperature);
      break;
    }
  }
}


/// @brief 5秒間隔で実行したい処理
void internal::task02Hz() {
  // そんなものはない
}


/// @brief 2Hzで実行したい処理
void internal::task2Hz() {
  // SDの検知
  if (device::detection::cardDetection.is(Var::SwitchState::CLOSE)) {
    // SDを検知した時はLED常時点灯
    device::indicator::sdStatus.on();
  }
  else {
    // SDが検知できない時はLED点滅
    device::indicator::sdStatus.toggle();
  }
}


/// @brief 20Hzで実行したい処理
void internal::task20Hz() {
  // 地磁気はセンサからのODRが20Hzなので20Hzで読み出す
  device::sensor::bno.getMagnetometer(&data::magnetometer_x_nT, &data::magnetometer_y_nT, &data::magnetometer_z_nT);
  // CAN送信が20Hzなので、外気温はそれに合わせて20Hzで読み出す
  device::sensor::thermistor.getTemperature(&data::outsideTemperature_degC);
  device::sensor::bme.getTemperature(&data::internalTemperature_degC);

  // CANにデータを流す
  canbus::can.sendScalar(CANSTM::Label::OUTSIDE_TEMPERATURE, data::outsideTemperature_degC);
  canbus::can.sendScalar(CANSTM::Label::INTERNAL_TEMPERATURE, data::internalTemperature_degC);
  canbus::can.sendScalar(CANSTM::Label::ALTITUDE, data::altitude_m);
  canbus::can.sendScalar(CANSTM::Label::CLIMB_RATE, data::climbRate_mps);
  canbus::can.sendVector3D(CANSTM::Label::ORIENTATION, data::orientation_x_deg, data::orientation_y_deg, data::orientation_z_deg);
  canbus::can.sendVector3D(CANSTM::Label::LINEAR_ACCELERATION, data::linear_acceleration_x_mps2, data::linear_acceleration_y_mps2, data::linear_acceleration_z_mps2);
  canbus::can.sendSensingStatus(
    internal::trajectory.getReferencePressure(),
    device::sensor::bno.isSystemCalibrated(),
    static_cast<uint8_t>(device::peripheral::logger.getUsage())
  );
  device::indicator::canSend.toggle();
}


/// @brief 50Hzで実行したい処理
void internal::task50Hz() {
  // 気圧と気温から高度を算出する
  // 内部的には落下検知の処理もやっている
  internal::trajectory.update(data::pressure_hPa, data::outsideTemperature_degC);

  data::altitude_m = internal::trajectory.getAltitude();
  data::climbRate_mps = internal::trajectory.getClimbRate();

  // CANにデータを流す
  // 安全のため、燃焼終了時高度27.37m以上でないと落下判定しない
  canbus::can.sendTrajectoryData(internal::trajectory.isFalling() && data::altitude_m >= 27.37);
  device::indicator::canSend.toggle();
}


/// @brief 100Hzで実行したい処理
void internal::task100Hz() {
  // BNO055からのデータは基本的に100Hzで読み出す
  device::sensor::bno.getAcceleration(&data::acceleration_x_mps2, &data::acceleration_y_mps2, &data::acceleration_z_mps2);
  device::sensor::bno.getGyroscope(&data::gyroscope_x_dps, &data::gyroscope_y_dps, &data::gyroscope_z_dps);
  device::sensor::bno.getOrientation(&data::orientation_x_deg, &data::orientation_y_deg, &data::orientation_z_deg);
  device::sensor::bno.getLinearAcceleration(&data::linear_acceleration_x_mps2, &data::linear_acceleration_y_mps2, &data::linear_acceleration_z_mps2);
  device::sensor::bno.getGravityVector(&data::gravity_x_mps2, &data::gravity_y_mps2, &data::gravity_z_mps2);
  device::sensor::bno.getQuaternion(&data::quaternion_w, &data::quaternion_x, &data::quaternion_y, &data::quaternion_z);
  // 高度も解析用にできるだけ早い100Hzで読み出したい
  device::sensor::bme.getPressure(&data::pressure_hPa);

  // doLoggingのフラグが立っている時はログを保存する
  // 内部的にはFRAMとSDに書き込んでいる
  if (device::peripheral::logger.isLogging()) {
    device::peripheral::logger.log(
      millis(), static_cast<uint8_t>(data::flightMode),
      data::outsideTemperature_degC, data::pressure_hPa, data::altitude_m, internal::trajectory.climbIndex(), internal::trajectory.isFalling(),
      data::acceleration_x_mps2, data::acceleration_y_mps2, data::acceleration_z_mps2,
      data::gyroscope_x_dps, data::gyroscope_y_dps, data::gyroscope_z_dps,
      data::magnetometer_x_nT, data::magnetometer_y_nT, data::magnetometer_z_nT,
      data::orientation_x_deg, data::orientation_y_deg, data::orientation_z_deg,
      data::linear_acceleration_x_mps2, data::linear_acceleration_y_mps2, data::linear_acceleration_z_mps2,
      data::gravity_x_mps2, data::gravity_y_mps2, data::gravity_z_mps2,
      data::quaternion_w, data::quaternion_x, data::quaternion_y, data::quaternion_z,
      data::collected_temperature, data::cold_junction_temperature, data::thermo_couple_temperature,
      data::internalTemperature_degC
    );
  }
}


/// @brief CANで受け取ったSystemStatusを使って処理を行う関数
///        loop()内のCAN受信処理から呼び出される用
void canbus::handleSystemStatus() {
  Var::State cameraState, sn3State;
  bool doLogging;
  uint16_t flightTime;
  uint8_t loggerUsage;

  canbus::can.receiveSystemStatus(&data::flightMode, &cameraState, &sn3State, &doLogging, &flightTime, &loggerUsage);

  // ログ保存を"やるはず"なのに"やっていない"なら開始
  if (doLogging && !device::peripheral::logger.isLogging()) {
    bool isSdDetected = device::detection::cardDetection.is(Var::SwitchState::CLOSE);
    bool isSucceeded = device::peripheral::logger.beginLogging(isSdDetected);

    if (!isSucceeded) {
      canbus::can.sendError(CANSTM::Publisher::SENSING_MODULE, CANSTM::ErrorCode::LOGGER_FAILURE, CANSTM::ErrorReason::INVALID_SD);
    }

    internal::trajectory.setReferencePressure(data::pressure_hPa);
    device::indicator::loggerStatus.on();
  }

  // ログ保存を"やらない"はずなのに"やっている"なら終了
  if (!doLogging && device::peripheral::logger.isLogging()) {
    device::peripheral::logger.endLogging();
    device::indicator::loggerStatus.off();
  }
}


/// @brief CANで受け取ったSetReferencePressureを使って処理を行う関数
///        loop()内のCAN受信処理から呼び出される用
void canbus::handleSetReferencePressure() {
  float newReferencePressure_hPa;

  canbus::can.receiveSetReferencePressure(&newReferencePressure_hPa);

  // 参照気圧を更新したことをイベントとして知らせる
  canbus::can.sendEvent(CANSTM::Publisher::SENSING_MODULE, CANSTM::EventCode::REFERENCE_PRESSURE_UPDATED);
  device::indicator::canSend.toggle();

  // 高度算出用のライブラリに新しい参照気圧を設定する
  internal::trajectory.setReferencePressure(newReferencePressure_hPa);
}