#include <TaskManager.h>
#include "CANSTM.hpp"
#include "BNO055.hpp"
#include "BME280.hpp"
#include "Thermistor.hpp"
#include "PullupPin.hpp"
#include "OutputPin.hpp"
#include "Trajectory.hpp"
#include "Blinker.hpp"
#include "Logger.hpp"
#include "Sd.hpp"


namespace timer {
  void task1Hz();
  void task20Hz();
  void task100Hz();
}

namespace sensor {
  BNO055 bno;
  BME bme;
  Thermistor thermistor(A1);
}

namespace logger {
  Logger logger(A2, A3);
  Sd sd(A0);
  PullupPin cardDetection(D8);

  bool doLogging;
}

namespace indicator {
  OutputPin canSend(D12);
  OutputPin canReceive(D11);
  Blinker sdStatus(D9, "invalidSd");
  OutputPin loggerStatus(D6);
}

namespace control {
  OutputPin recorderPower(D7);
}

namespace connection {
  CANSTM can;

  void handleSystemStatus();
  void handleSetReferencePressure();
}

namespace data {
  Trajectory trajectory;

  float pressure_hPa;
  float outsideTemperature_degC;
  float altitude_m;

  float acceleration_x_mps2, acceleration_y_mps2, acceleration_z_mps2;
  float magnetometer_x_nT, magnetometer_y_nT, magnetometer_z_nT;
  float gyroscope_x_dps, gyroscope_y_dps, gyroscope_z_dps;
  float orientation_x_deg, orientation_y_deg, orientation_z_deg;
  float linear_acceleration_x_mps2, linear_acceleration_y_mps2, linear_acceleration_z_mps2;
  float gravity_x_mps2, gravity_y_mps2, gravity_z_mps2;

  uint8_t mode;
  bool camera, separator;
}

namespace develop {
  PullupPin debugMode(D3);

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

  // FRAMとSDの電源は常にON
  control::recorderPower.on();

  SPI.setMOSI(A6);
  SPI.setMISO(A5);
  SPI.setSCLK(A4);
  SPI.begin();

  // SDの初期検知と初期化
  if (logger::sd.begin()) {
    indicator::sdStatus.on();
  }
  else {
    indicator::sdStatus.startBlink(2);
  }

  Wire.setSDA(D4);
  Wire.setSCL(D5);
  Wire.begin();
  Wire.setClock(400000);

  sensor::bno.begin();
  sensor::bme.begin();
  sensor::thermistor.initialize();

  connection::can.begin();
  connection::can.sendEvent(CANSTM::Publisher::SENSING_MODULE, CANSTM::EventCode::SETUP);

  Tasks.add(timer::task20Hz)->startFps(20);
  Tasks.add(timer::task100Hz)->startFps(100);
}


void loop() {
  Tasks.update();

  //CAN受信処理
  if (connection::can.available()) {
    switch (connection::can.getLatestMessageLabel()) {
    case CANSTM::Label::SYSTEM_STATUS:
      connection::handleSystemStatus();
      break;
    case CANSTM::Label::SET_REFERENCE_PRESSURE:
      connection::handleSetReferencePressure();
      break;
    }
  }
}


/// @brief 1Hzで実行したい処理
void timer::task1Hz() {
  // SDの検知の更新
  // SDを新しく検知した時
  if (!logger::doLogging && !logger::sd.isRunning() && !logger::cardDetection.isOpen()) {
    logger::sd.begin();
    indicator::sdStatus.stopBlink();
    indicator::sdStatus.on();
  }

  // SDが検知できなくなった時
  if (!logger::doLogging && logger::sd.isRunning() && logger::cardDetection.isOpen()) {
    logger::sd.end();
    indicator::sdStatus.startBlink(2);
  }
}


/// @brief 20Hzで実行したい処理
void timer::task20Hz() {
  // 地磁気はセンサからのODRが20Hzなので20Hzで読み出す
  sensor::bno.getMagnetometer(&data::magnetometer_x_nT, &data::magnetometer_y_nT, &data::magnetometer_z_nT);
  // CAN送信が20Hzなので、外気温はそれに合わせて20Hzで読み出す
  sensor::thermistor.getTemperature(&data::outsideTemperature_degC);

  // 気圧と気温から高度を算出する
  // 内部的には落下検知の処理もやっている
  data::altitude_m = data::trajectory.update(data::pressure_hPa, data::outsideTemperature_degC);

  // CANにデータを流す
    // 安全のため、高度50m以上でないと落下判定しない
  connection::can.sendTrajectoryData(data::trajectory.isFalling() && data::altitude_m >= 50.0);
  connection::can.sendScalar(CANSTM::Label::OUTSIDE_TEMPERATURE, data::outsideTemperature_degC);
  connection::can.sendScalar(CANSTM::Label::ALTITUDE, data::altitude_m);
  connection::can.sendVector3D(CANSTM::Label::ORIENTATION, data::magnetometer_x_nT, data::magnetometer_y_nT, data::magnetometer_z_nT);
  connection::can.sendVector3D(CANSTM::Label::LINEAR_ACCELERATION, data::linear_acceleration_x_mps2, data::linear_acceleration_y_mps2, data::linear_acceleration_z_mps2);
  indicator::canSend.toggle();
}


/// @brief 100Hzで実行したい処理
void timer::task100Hz() {
  // BNO055からのデータは基本的に100Hzで読み出す
  sensor::bno.getAcceleration(&data::acceleration_x_mps2, &data::acceleration_y_mps2, &data::acceleration_z_mps2);
  sensor::bno.getGyroscope(&data::gyroscope_x_dps, &data::gyroscope_y_dps, &data::gyroscope_z_dps);
  sensor::bno.getOrientation(&data::orientation_x_deg, &data::orientation_y_deg, &data::orientation_z_deg);
  sensor::bno.getLinearAcceleration(&data::linear_acceleration_x_mps2, &data::linear_acceleration_y_mps2, &data::linear_acceleration_z_mps2);
  sensor::bno.getGravityVector(&data::gravity_x_mps2, &data::gravity_y_mps2, &data::gravity_z_mps2);
  // 高度も解析用にできるだけ早い100Hzで読み出したい
  sensor::bme.getPressure(&data::pressure_hPa);


  // doLoggingのフラグが立っている時はログを保存する
  // 内部的にはFRAMとSDに書き込んでいる
  if (logger::doLogging) {
    logger::logger.log(
      millis(),
      data::outsideTemperature_degC, data::pressure_hPa, data::altitude_m, data::trajectory.climbIndex(), data::trajectory.isFalling(),
      data::acceleration_x_mps2, data::acceleration_y_mps2, data::acceleration_z_mps2,
      data::gyroscope_x_dps, data::gyroscope_y_dps, data::gyroscope_z_dps,
      data::magnetometer_x_nT, data::magnetometer_y_nT, data::magnetometer_z_nT,
      data::orientation_x_deg, data::orientation_y_deg, data::orientation_z_deg,
      data::linear_acceleration_x_mps2, data::linear_acceleration_y_mps2, data::linear_acceleration_z_mps2,
      data::gravity_x_mps2, data::gravity_y_mps2, data::gravity_z_mps2
    );
  }
}


/// @brief CANで受け取ったSystemStatusを使って処理を行う関数
///        loop()内のCAN受信処理から呼び出される用
void connection::handleSystemStatus() {
  connection::can.receiveSystemStatus(&data::mode, &data::camera, &data::separator);
  indicator::canReceive.toggle();

  // フライトモードがSTANDBYとLANDの間ならログを保存する
  if (1 <= data::mode && data::mode <= 7) {
    if (!logger::doLogging) {
      logger::doLogging = true;
      logger::logger.reset();
      indicator::loggerStatus.on();
    }
  }
  else {
    if (logger::doLogging) {
      logger::doLogging = false;
      indicator::loggerStatus.off();
    }
  }
}


/// @brief CANで受け取ったSetReferencePressureを使って処理を行う関数
///        loop()内のCAN受信処理から呼び出される用
void connection::handleSetReferencePressure() {
  float newReferencePressure_hPa;

  connection::can.receiveSetReferencePressure(&newReferencePressure_hPa);
  indicator::canReceive.toggle();

  // 参照気圧を更新したことをイベントとして知らせる
  connection::can.sendEvent(CANSTM::Publisher::SENSING_MODULE, CANSTM::EventCode::REFERENCE_PRESSURE_UPDATED);
  indicator::canSend.toggle();

  // 高度算出用のライブラリに新しい参照気圧を設定する
  data::trajectory.setReferencePressure(newReferencePressure_hPa);
}