#include <Wire.h>
#include <TaskManager.h>
#include "CANBUS.hpp"
#include "BNO055.hpp"
#include "Thermistor.hpp"


CANBUS canbus;

BNO055 bno055(&Wire, 0x28);
Thermistor thermistor1(PA_2);
Thermistor thermistor2(PA_3);


double acceleration_x, acceleration_y, acceleration_z;
double magnetometer_x, magnetometer_y, magnetometer_z;
double gyroscope_x, gyroscope_y, gyroscope_z;
double euler_heading, euler_roll, euler_pitch;
double quaternion_w, quaternion_x, quaternion_y, quaternion_z;
double linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
double gravity_x, gravity_y, gravity_z;

double temperature1;
double temperature2;


void setup() {
  analogReadResolution(12);

  Serial.begin(115200);

  Wire.setSDA(PB_7);
  Wire.setSCL(PB_6);
  Wire.begin();
  Wire.setClock(400000);

  canbus.initialize();

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
  canbus.sendMessage();
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