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
  void handleSetReferencePressureCommand();
}

namespace data {
  Trajectory trajectory(0.25, 0.75);

  float pressure;
  float temperature;
  float altitude;

  float acceleration_x, acceleration_y, acceleration_z;
  float magnetometer_x, magnetometer_y, magnetometer_z;
  float gyroscope_x, gyroscope_y, gyroscope_z;
  float orientation_x, orientation_y, orientation_z;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
  float gravity_x, gravity_y, gravity_z;

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

  //CAN受信処理
  if (connection::can.available()) {
    switch (connection::can.getLatestMessageLabel()) {
    case CANSTM::Label::SYSTEM_STATUS:
      connection::handleSystemStatus();
      break;
    case CANSTM::Label::SET_REFERENCE_PRESSURE_COMMAND:
      connection::handleSetReferencePressureCommand();
      break;
    }
  }
}


void timer::task20Hz() {
  sensor::bno.getMagnetometer(&data::magnetometer_x, &data::magnetometer_y, &data::magnetometer_z);
  sensor::thermistor.getTemperature(&data::temperature);

  data::altitude = data::trajectory.update(data::pressure, data::temperature);

  connection::can.sendScalar(CANSTM::Label::TEMPERATURE, data::temperature);
  connection::can.sendScalar(CANSTM::Label::ALTITUDE, data::altitude);
  connection::can.sendTrajectoryData(data::trajectory.isFalling());
  connection::can.sendVector3D(CANSTM::Label::ORIENTATION, data::magnetometer_x, data::magnetometer_y, data::magnetometer_z);
  connection::can.sendVector3D(CANSTM::Label::LINEAR_ACCELERATION, data::linear_acceleration_x, data::linear_acceleration_y, data::linear_acceleration_z);
  indicator::canSend.toggle();
}


void timer::task100Hz() {
  sensor::bno.getAcceleration(&data::acceleration_x, &data::acceleration_y, &data::acceleration_z);
  sensor::bno.getGyroscope(&data::gyroscope_x, &data::gyroscope_y, &data::gyroscope_z);
  sensor::bno.getOrientation(&data::orientation_x, &data::orientation_y, &data::orientation_z);
  sensor::bno.getLinearAcceleration(&data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
  sensor::bno.getGravityVector(&data::gravity_x, &data::gravity_y, &data::gravity_z);
  sensor::bme.getPressure(&data::pressure);


  if (logger::doLogging) {
    logger::logger.log(
      millis(),
      data::temperature, data::pressure, data::altitude, data::trajectory.climbIndex(), data::trajectory.isFalling(),
      data::acceleration_x, data::acceleration_y, data::acceleration_z,
      data::gyroscope_x, data::gyroscope_y, data::gyroscope_z,
      data::magnetometer_x, data::magnetometer_y, data::magnetometer_z,
      data::orientation_x, data::orientation_y, data::orientation_z,
      data::linear_acceleration_x, data::linear_acceleration_y, data::linear_acceleration_z,
      data::gravity_x, data::gravity_y, data::gravity_z
    );
  }
}


void connection::handleSystemStatus() {
  connection::can.receiveStatus(&data::mode, &data::camera, &data::separator);
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


void connection::handleSetReferencePressureCommand() {
  float newReferencePressure;

  connection::can.receiveSetReferencePressureCommand(&newReferencePressure);
  indicator::canReceive.toggle();

  connection::can.sendEvent(CANSTM::Publisher::SENSING_MODULE, CANSTM::EventCode::REFERENCE_PRESSURE_UPDATED);
  indicator::canSend.toggle();

  data::trajectory.setReferencePressure(newReferencePressure);
}