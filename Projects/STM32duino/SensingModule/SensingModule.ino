#include <ACAN_STM32.h>
#include <TaskManager.h>
#include "BNO055.hpp"
#include "BME280.hpp"
#include "Thermistor.hpp"
#include "PullupPin.hpp"
#include "OutputPin.hpp"
#include "FRAM.hpp"
#include "Sd.hpp"


namespace canbus {
  enum class Id : uint32_t {
    TEMPERATURE,
    PRESSURE,
    ALTITUDE,
    ACCELERATION,
    GYROSCOPE,
    MAGNETOMETER,
    ORIENTATION,
    LINEAR_ACCELERATION,
    GRAVITY,
    STATUS
  };

  enum class Axis : uint8_t {
    X,
    Y,
    Z
  };

  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  void initialize();
  void sendScalar(canbus::Id id, float value);
  void sendVector(canbus::Id id, canbus::Axis axis, float value);
  void receiveStatus(CANMessage message);
}

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
}

namespace timer {
  void task10Hz();
  void task20Hz();
  void task100Hz();

  void invalidSdBlink();
}

namespace sensor {
  BNO055 bno;
  BME bme;
  Thermistor thermistor(A1);
}

namespace recorder {
  FRAM fram0(A2);
  FRAM fram1(A3);
  Sd sd(A0);

  PullupPin cardDetection(D8);

  bool doRecording;
}

namespace indicator {
  OutputPin canSend(D12);
  OutputPin canReceive(D11);

  OutputPin sdStatus(D9);
  OutputPin recorderStatus(D6);
}

namespace control {
  OutputPin recorderPower(D7);
}

namespace data {
  float referencePressure = 1013.25;

  float pressure;
  float temperature;
  float altitude;

  float acceleration_x, acceleration_y, acceleration_z;
  float magnetometer_x, magnetometer_y, magnetometer_z;
  float gyroscope_x, gyroscope_y, gyroscope_z;
  float orientation_x, orientation_y, orientation_z;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
  float gravity_x, gravity_y, gravity_z;
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
  }


  // 開発中: 保存は常に行う表示
  control::recorderPower.on();
  indicator::recorderStatus.on();

  SPI.setMOSI(A6);
  SPI.setMISO(A5);
  SPI.setSCLK(A4);
  SPI.begin();

  if (recorder::sd.begin()) {
    indicator::sdStatus.on();
  }
  else {
    Tasks.add("invalidSdBlink", timer::invalidSdBlink)->startIntervalMsec(500);
  }

  Wire.setSDA(D4);
  Wire.setSCL(D5);
  Wire.begin();
  Wire.setClock(400000);

  sensor::bno.begin();
  sensor::bme.begin();
  sensor::thermistor.initialize();

  canbus::initialize();

  Tasks.add(timer::task10Hz)->startFps(10);
  Tasks.add(timer::task20Hz)->startFps(20);
  Tasks.add(timer::task100Hz)->startFps(100);

  // メモリ切断処理
  // control::recorderPower.off();
  // SPI.end();
  // pinMode(A6, OUTPUT); // MOSI
  // digitalWrite(A6, LOW);
  // pinMode(A5, OUTPUT); // MISO
  // digitalWrite(A5, LOW);
  // digitalWrite(A2, LOW); // CS FRAM0
  // digitalWrite(A3, LOW); // CS FRAM1
  // digitalWrite(A0, LOW); // CS SD
}


void loop() {
  Tasks.update();

  // SDの検知の更新
  // SDを新しく検知した時
  if (!recorder::doRecording && !recorder::sd.isRunning() && !recorder::cardDetection.isOpen()) {
    recorder::sd.begin();
    Tasks.erase("invalidSdBlink");
    indicator::sdStatus.on();
  }

  // SDが検知できなくなった時
  if (!recorder::doRecording && recorder::sd.isRunning() && recorder::cardDetection.isOpen()) {
    recorder::sd.end();
    Tasks.add("invalidSdBlink", timer::invalidSdBlink)->startFps(2);
  }

  if (can.available0()) {
    CANMessage message;
    can.receive0(message);

    switch (message.id) {
    case static_cast<uint32_t>(canbus::Id::STATUS):
      canbus::receiveStatus(message);
      break;
    }
  }
}


void canbus::initialize() {
  ACAN_STM32_Settings settings(500000);
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);
}


void canbus::sendScalar(canbus::Id id, float value) {
  CANMessage message;
  message.id = static_cast<uint32_t>(id);
  message.len = 4;

  canbus::converter.value = value;
  message.data[0] = canbus::converter.data[0];
  message.data[1] = canbus::converter.data[1];
  message.data[2] = canbus::converter.data[2];
  message.data[3] = canbus::converter.data[3];

  can.tryToSendReturnStatus(message);

  indicator::canSend.toggle();
}


void canbus::sendVector(canbus::Id id, canbus::Axis axis, float value) {
  CANMessage message;
  message.id = static_cast<uint32_t>(id);
  message.len = 5;
  message.data[0] = static_cast<uint8_t>(axis);

  canbus::converter.value = value;
  message.data[1] = canbus::converter.data[0];
  message.data[2] = canbus::converter.data[1];
  message.data[3] = canbus::converter.data[2];
  message.data[4] = canbus::converter.data[3];

  can.tryToSendReturnStatus(message);

  indicator::canSend.toggle();
}


void canbus::receiveStatus(CANMessage message) {
  flightMode::Mode mode = static_cast<flightMode::Mode>(message.data[0]);

  recorder::doRecording =
    mode == flightMode::Mode::STANDBY
    || mode == flightMode::Mode::THRUST
    || mode == flightMode::Mode::CLIMB
    || mode == flightMode::Mode::DESCENT
    || mode == flightMode::Mode::DECEL
    || mode == flightMode::Mode::PARACHUTE
    || mode == flightMode::Mode::LAND;

  indicator::canReceive.toggle();
}


void timer::task10Hz() {
  sensor::thermistor.getTemperature(&data::temperature);
}


void timer::task20Hz() {
  sensor::bno.getMagnetometer(&data::magnetometer_x, &data::magnetometer_y, &data::magnetometer_z);

  canbus::sendVector(canbus::Id::ORIENTATION, canbus::Axis::X, data::orientation_x);
  canbus::sendVector(canbus::Id::ORIENTATION, canbus::Axis::Y, data::orientation_y);
  canbus::sendVector(canbus::Id::ORIENTATION, canbus::Axis::Z, data::orientation_z);

  canbus::sendVector(canbus::Id::LINEAR_ACCELERATION, canbus::Axis::X, data::linear_acceleration_x);
  canbus::sendVector(canbus::Id::LINEAR_ACCELERATION, canbus::Axis::Y, data::linear_acceleration_y);
  canbus::sendVector(canbus::Id::LINEAR_ACCELERATION, canbus::Axis::Z, data::linear_acceleration_z);
}


void timer::task100Hz() {
  sensor::bno.getAcceleration(&data::acceleration_x, &data::acceleration_y, &data::acceleration_z);
  sensor::bno.getGyroscope(&data::gyroscope_x, &data::gyroscope_y, &data::gyroscope_z);
  sensor::bno.getOrientation(&data::orientation_x, &data::orientation_y, &data::orientation_z);
  sensor::bno.getLinearAcceleration(&data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
  sensor::bno.getGravityVector(&data::gravity_x, &data::gravity_y, &data::gravity_z);
  sensor::bme.getPressure(&data::pressure);

  data::altitude = (((pow((data::referencePressure / data::pressure), (1.0 / 5.257))) - 1.0) * (data::temperature + 273.15)) / 0.0065;

  canbus::sendScalar(canbus::Id::ALTITUDE, data::altitude);
}


void timer::invalidSdBlink() {
  indicator::sdStatus.toggle();
}