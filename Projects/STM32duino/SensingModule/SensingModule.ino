#include <ACAN_STM32.h>
#include <TaskManager.h>
#include "BNO055.hpp"
#include "LPS33HW.hpp"
#include "Thermistor.hpp"


#undef EULER


namespace canbus {
  enum class Id {
    TEMPERATURE,
    PRESSURE,
    ACCELERATION,
    GYROSCOPE,
    MAGNETOMETER,
    EULER,
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


BNO055 bno;
float acceleration_x, acceleration_y, acceleration_z;
float magnetometer_x, magnetometer_y, magnetometer_z;
float gyroscope_x, gyroscope_y, gyroscope_z;
float euler_x, euler_y, euler_z;
float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
float gravity_x, gravity_y, gravity_z;

LPS33HW lps;
float pressure;

Thermistor thermistor(PA_2);
float temperature;


void setup() {
  analogReadResolution(12);

  Serial.begin(115200);

  Wire.setSDA(PB_7);
  Wire.setSCL(PB_6);
  Wire.begin();
  Wire.setClock(400000);

  bno.begin();
  lps.begin();
  thermistor.initialize();

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
  thermistor.getTemperature(&temperature);
  canbus::sendNorm(canbus::Id::TEMPERATURE, temperature);
}


void task20Hz() {
  bno.getMagnetometer(&magnetometer_x, &magnetometer_y, &magnetometer_z);
  canbus::sendVector(canbus::Id::MAGNETOMETER, canbus::Axis::X, magnetometer_x);
  canbus::sendVector(canbus::Id::MAGNETOMETER, canbus::Axis::Y, magnetometer_y);
  canbus::sendVector(canbus::Id::MAGNETOMETER, canbus::Axis::Z, magnetometer_z);
}


void task50Hz() {
  lps.getPressure(&pressure);
  canbus::sendNorm(canbus::Id::PRESSURE, pressure);
}


void task100Hz() {
  bno.getAcceleration(&acceleration_x, &acceleration_y, &acceleration_z);
  canbus::sendVector(canbus::Id::ACCELERATION, canbus::Axis::X, acceleration_x);
  canbus::sendVector(canbus::Id::ACCELERATION, canbus::Axis::Y, acceleration_y);
  canbus::sendVector(canbus::Id::ACCELERATION, canbus::Axis::Z, acceleration_z);

  bno.getGyroscope(&gyroscope_x, &gyroscope_y, &gyroscope_z);
  canbus::sendVector(canbus::Id::GYROSCOPE, canbus::Axis::X, gyroscope_x);
  canbus::sendVector(canbus::Id::GYROSCOPE, canbus::Axis::Y, gyroscope_y);
  canbus::sendVector(canbus::Id::GYROSCOPE, canbus::Axis::Z, gyroscope_z);

  bno.getEuler(&euler_x, &euler_y, &euler_z);
  canbus::sendVector(canbus::Id::EULER, canbus::Axis::X, euler_x);
  canbus::sendVector(canbus::Id::EULER, canbus::Axis::Y, euler_y);
  canbus::sendVector(canbus::Id::EULER, canbus::Axis::Z, euler_z);

  bno.getLinearAcceleration(&linear_acceleration_x, &linear_acceleration_y, &linear_acceleration_z);
  canbus::sendVector(canbus::Id::LINEAR_ACCELERATION, canbus::Axis::X, linear_acceleration_x);
  canbus::sendVector(canbus::Id::LINEAR_ACCELERATION, canbus::Axis::Y, linear_acceleration_y);
  canbus::sendVector(canbus::Id::LINEAR_ACCELERATION, canbus::Axis::Z, linear_acceleration_z);

  bno.getGravityVector(&gravity_x, &gravity_y, &gravity_z);
  canbus::sendVector(canbus::Id::GRAVITY, canbus::Axis::X, gravity_x);
  canbus::sendVector(canbus::Id::GRAVITY, canbus::Axis::Y, gravity_y);
  canbus::sendVector(canbus::Id::GRAVITY, canbus::Axis::Z, gravity_z);
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