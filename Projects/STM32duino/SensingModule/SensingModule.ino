#include <Wire.h>
#include <TaskManager.h>
#include <ACAN_STM32.h>
#include "BNO055.hpp"
#include "Thermistor.hpp"


BNO055 bno055(&Wire, 0x28);
double acceleration_x, acceleration_y, acceleration_z;
double magnetometer_x, magnetometer_y, magnetometer_z;
double gyroscope_x, gyroscope_y, gyroscope_z;
double euler_heading, euler_roll, euler_pitch;
double quaternion_w, quaternion_x, quaternion_y, quaternion_z;
double linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
double gravity_x, gravity_y, gravity_z;

Thermistor thermistor1(PA_2);
double temperature1;

Thermistor thermistor2(PA_3);
double temperature2;


void setup() {
  analogReadResolution(12);

  Serial.begin(115200);

  Wire.setSDA(PB_7);
  Wire.setSCL(PB_6);
  Wire.begin();
  Wire.setClock(400000);

  bno055.initialize();
  thermistor1.initialize();
  thermistor2.initialize();

  pinMode(LED_BUILTIN, OUTPUT);
  ACAN_STM32_Settings settings(125 * 1000); // 125 kbit/s
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);

  Tasks.add(task1Hz)->startIntervalMsec(10);
  Tasks.add(task10Hz)->startIntervalMsec(100);
  Tasks.add(task20Hz)->startIntervalMsec(50);
  Tasks.add(task100Hz)->startIntervalMsec(10);
}


void loop() {
  Tasks.update();
}


void task1Hz() {
  CANMessage message;
  message.id = 0xCA;
  message.len = 4;
  uint32_t valueRaw = (uint32_t)(temperature1 * 4096.0);
  message.data[0] = (uint8_t)((valueRaw & 0xFF000000) >> 24);
  message.data[1] = (uint8_t)((valueRaw & 0x00FF0000) >> 16);
  message.data[2] = (uint8_t)((valueRaw & 0x0000FF00) >> 8);
  message.data[3] = (uint8_t)((valueRaw & 0x000000FF) >> 0);

  const bool ok = can.tryToSendReturnStatus(message);
  Serial.print("Sended: ");
  Serial.print(message.data[0], HEX);
  Serial.print(" ");
  Serial.print(message.data[1], HEX);
  Serial.print(" ");
  Serial.print(message.data[2], HEX);
  Serial.print(" ");
  Serial.println(message.data[3], HEX);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}


void task10Hz() {
  thermistor1.getTemperature(&temperature1);
  thermistor2.getTemperature(&temperature2);
}


void task20Hz() {
  bno055.getMagnetometer(&magnetometer_x, &magnetometer_y, &magnetometer_z);
}


void task100Hz() {
  bno055.getAcceleration(&acceleration_x, &acceleration_y, &acceleration_z);
  bno055.getGyroscope(&gyroscope_x, &gyroscope_y, &gyroscope_z);
  bno055.getEuler(&euler_heading, &euler_roll, &euler_pitch);
  bno055.getQuaternion(&quaternion_w, &quaternion_x, &quaternion_y, &quaternion_z);
  bno055.getLinearAcceleration(&linear_acceleration_x, &linear_acceleration_y, &linear_acceleration_z);
  bno055.getGravityVector(&gravity_x, &gravity_y, &gravity_z);
}