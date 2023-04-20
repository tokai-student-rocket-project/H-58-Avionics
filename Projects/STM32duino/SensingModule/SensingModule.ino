#include <ACAN_STM32.h>
#include <TaskManager.h>
#include "BNO055.hpp"
#include "LPS33HW.hpp"
#include "Thermistor.hpp"


namespace canbus {
  enum class Id {
    TEMPERATURE,
    PRESSURE,
    ACCELERATION,
    GYROSCOPE,
    MAGNETOMETER,
    ORIENTATION,
    LINEAR_ACCELERATION,
    GRAVITY
  };

  enum class Axis {
    X,
    Y,
    Z
  };

  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  void sendNorm(canbus::Id id, float value);
  void sendVector(canbus::Id id, canbus::Axis axis, float value);
}

namespace device {
  BNO055 bno;
  LPS33HW lps;
  Thermistor thermistor(PA_2);
}

namespace data {
  float acceleration_x, acceleration_y, acceleration_z;
  float magnetometer_x, magnetometer_y, magnetometer_z;
  float gyroscope_x, gyroscope_y, gyroscope_z;
  float orientation_x, orientation_y, orientation_z;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
  float gravity_x, gravity_y, gravity_z;
  float pressure;
  float temperature;
}


void setup() {
  analogReadResolution(12);

  Serial.begin(115200);

  Wire.setSDA(PB_7);
  Wire.setSCL(PB_6);
  Wire.begin();
  Wire.setClock(400000);

  device::bno.begin();
  device::lps.begin();
  device::thermistor.initialize();

  ACAN_STM32_Settings settings(1000000);
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);

  Tasks.add(task2Hz)->startIntervalMsec(500);
  Tasks.add(task20Hz)->startIntervalMsec(50);
  Tasks.add(task50Hz)->startIntervalMsec(20);
  Tasks.add(task100Hz)->startIntervalMsec(10);
}


void loop() {
  Tasks.update();
}


void task2Hz() {
  device::thermistor.getTemperature(&data::temperature);
  canbus::sendNorm(canbus::Id::TEMPERATURE, data::temperature);
}


void task20Hz() {
  device::bno.getMagnetometer(&data::magnetometer_x, &data::magnetometer_y, &data::magnetometer_z);
  canbus::sendVector(canbus::Id::MAGNETOMETER, canbus::Axis::X, data::magnetometer_x);
  canbus::sendVector(canbus::Id::MAGNETOMETER, canbus::Axis::Y, data::magnetometer_y);
  canbus::sendVector(canbus::Id::MAGNETOMETER, canbus::Axis::Z, data::magnetometer_z);
}


void task50Hz() {
  device::lps.getPressure(&data::pressure);
  canbus::sendNorm(canbus::Id::PRESSURE, data::pressure);
}


void task100Hz() {
  device::bno.getAcceleration(&data::acceleration_x, &data::acceleration_y, &data::acceleration_z);
  canbus::sendVector(canbus::Id::ACCELERATION, canbus::Axis::X, data::acceleration_x);
  canbus::sendVector(canbus::Id::ACCELERATION, canbus::Axis::Y, data::acceleration_y);
  canbus::sendVector(canbus::Id::ACCELERATION, canbus::Axis::Z, data::acceleration_z);

  device::bno.getGyroscope(&data::gyroscope_x, &data::gyroscope_y, &data::gyroscope_z);
  canbus::sendVector(canbus::Id::GYROSCOPE, canbus::Axis::X, data::gyroscope_x);
  canbus::sendVector(canbus::Id::GYROSCOPE, canbus::Axis::Y, data::gyroscope_y);
  canbus::sendVector(canbus::Id::GYROSCOPE, canbus::Axis::Z, data::gyroscope_z);

  device::bno.getOrientation(&data::orientation_x, &data::orientation_y, &data::orientation_z);
  canbus::sendVector(canbus::Id::ORIENTATION, canbus::Axis::X, data::orientation_x);
  canbus::sendVector(canbus::Id::ORIENTATION, canbus::Axis::Y, data::orientation_y);
  canbus::sendVector(canbus::Id::ORIENTATION, canbus::Axis::Z, data::orientation_z);

  device::bno.getLinearAcceleration(&data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
  canbus::sendVector(canbus::Id::LINEAR_ACCELERATION, canbus::Axis::X, data::linear_acceleration_x);
  canbus::sendVector(canbus::Id::LINEAR_ACCELERATION, canbus::Axis::Y, data::linear_acceleration_y);
  canbus::sendVector(canbus::Id::LINEAR_ACCELERATION, canbus::Axis::Z, data::linear_acceleration_z);

  device::bno.getGravityVector(&data::gravity_x, &data::gravity_y, &data::gravity_z);
  canbus::sendVector(canbus::Id::GRAVITY, canbus::Axis::X, data::gravity_x);
  canbus::sendVector(canbus::Id::GRAVITY, canbus::Axis::Y, data::gravity_y);
  canbus::sendVector(canbus::Id::GRAVITY, canbus::Axis::Z, data::gravity_z);
}


void canbus::sendNorm(canbus::Id id, float value) {
  CANMessage message;
  message.id = static_cast<uint8_t>(id);
  message.len = 4;

  canbus::converter.value = value;
  message.data[0] = canbus::converter.data[0];
  message.data[1] = canbus::converter.data[1];
  message.data[2] = canbus::converter.data[2];
  message.data[3] = canbus::converter.data[3];

  can.tryToSendReturnStatus(message);
}


void canbus::sendVector(canbus::Id id, canbus::Axis axis, float value) {
  CANMessage message;
  message.id = static_cast<uint8_t>(id);
  message.len = 5;
  message.data[0] = static_cast<uint8_t>(axis);

  canbus::converter.value = value;
  message.data[1] = canbus::converter.data[0];
  message.data[2] = canbus::converter.data[1];
  message.data[3] = canbus::converter.data[2];
  message.data[4] = canbus::converter.data[3];

  can.tryToSendReturnStatus(message);
}