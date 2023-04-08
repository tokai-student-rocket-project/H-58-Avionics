#include <Wire.h>
#include <TaskManager.h>
#include "CanBus.hpp"
#include "BNO055.hpp"
#include "Thermistor.hpp"


CanBusNamespace::CanBus can;
CanBusNamespace::Frame tx;

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

  bool ret = can.begin(CanBusNamespace::Bitrate::CAN_1000KBPS, CanBusNamespace::Pinout::RX_PA11_TX_PA12);
  Serial.println(ret ? "BOOT OK" : "BOOT FAILURE");
  if (!ret) while (true);

  bno055.initialize();
  thermistor1.initialize();
  thermistor2.initialize();

  Tasks.add(task1Hz)->startIntervalMsec(1000);
  Tasks.add(task10Hz)->startIntervalMsec(100);
  Tasks.add(task20Hz)->startIntervalMsec(50);
  Tasks.add(task100Hz)->startIntervalMsec(10);
}


void loop() {
  Tasks.update();
}


void task1Hz() {
  tx = { 0x17FF, 3, {0x01, 0x02, 0x03}, CanBusNamespace::FrameFormat::Extended, CanBusNamespace::FrameType::Data };
  can.send(&tx);
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