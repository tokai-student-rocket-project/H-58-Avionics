#include <ACAN_STM32.h>
#include <TaskManager.h>
#include "BNO055.hpp"
#include "LPS33HW.hpp"
#include "Thermistor.hpp"


namespace canbus {
  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  void sendNorm(uint32_t id, float value);
  void sendVector(uint32_t id, uint8_t axis, float value);
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
  canbus::sendNorm(0x00, temperature);
}


void task20Hz() {
  bno.getMagnetometer(&magnetometer_x, &magnetometer_y, &magnetometer_z);
  canbus::sendVector(0x04, 0, magnetometer_x);
  canbus::sendVector(0x04, 1, magnetometer_y);
  canbus::sendVector(0x04, 2, magnetometer_z);
}


void task50Hz() {
  lps.getPressure(&pressure);
  canbus::sendNorm(0x01, pressure);
}


void task100Hz() {
  bno.getAcceleration(&acceleration_x, &acceleration_y, &acceleration_z);
  canbus::sendVector(0x02, 0, acceleration_x);
  canbus::sendVector(0x02, 1, acceleration_y);
  canbus::sendVector(0x02, 2, acceleration_z);

  bno.getGyroscope(&gyroscope_x, &gyroscope_y, &gyroscope_z);
  canbus::sendVector(0x03, 0, gyroscope_x);
  canbus::sendVector(0x03, 1, gyroscope_y);
  canbus::sendVector(0x03, 2, gyroscope_z);

  bno.getEuler(&euler_x, &euler_y, &euler_z);
  canbus::sendVector(0x05, 0, euler_x);
  canbus::sendVector(0x05, 1, euler_y);
  canbus::sendVector(0x05, 2, euler_z);

  bno.getLinearAcceleration(&linear_acceleration_x, &linear_acceleration_y, &linear_acceleration_z);
  canbus::sendVector(0x06, 0, linear_acceleration_x);
  canbus::sendVector(0x06, 1, linear_acceleration_y);
  canbus::sendVector(0x06, 2, linear_acceleration_z);

  bno.getGravityVector(&gravity_x, &gravity_y, &gravity_z);
  canbus::sendVector(0x07, 0, gravity_x);
  canbus::sendVector(0x07, 1, gravity_y);
  canbus::sendVector(0x07, 2, gravity_z);
}


void canbus::sendNorm(uint32_t id, float value) {
  CANMessage message;
  message.id = id;
  message.len = 4;

  canbus::converter.value = value;
  message.data[0] = canbus::converter.data[0];
  message.data[1] = canbus::converter.data[1];
  message.data[2] = canbus::converter.data[2];
  message.data[3] = canbus::converter.data[3];

  can.tryToSendReturnStatus(message);
}


void canbus::sendVector(uint32_t id, uint8_t axis, float value) {
  CANMessage message;
  message.id = id;
  message.len = 5;
  message.data[0] = axis;

  canbus::converter.value = value;
  message.data[1] = canbus::converter.data[0];
  message.data[2] = canbus::converter.data[1];
  message.data[3] = canbus::converter.data[2];
  message.data[4] = canbus::converter.data[3];

  can.tryToSendReturnStatus(message);
}