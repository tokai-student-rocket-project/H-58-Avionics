#include <Wire.h>
#include <TaskManager.h>
#include "CANBUS.hpp"
#include "BNO055.hpp"
#include "LPS33HW.hpp"
#include "Thermistor.hpp"

CANBUS canbus;

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

float basePressure = 1013.25;
float altitude;

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

  canbus.initialize();

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
}


void task20Hz() {
  bno.getMagnetometer(&magnetometer_x, &magnetometer_y, &magnetometer_z);
}


void task50Hz() {
  lps.getPressure(&pressure);
  altitude = calculateAltitude(pressure, basePressure, temperature);

  Serial.println(altitude);
}


void task100Hz() {
  bno.getAcceleration(&acceleration_x, &acceleration_y, &acceleration_z);
  bno.getGyroscope(&gyroscope_x, &gyroscope_y, &gyroscope_z);
  bno.getEuler(&euler_x, &euler_y, &euler_z);
  bno.getLinearAcceleration(&linear_acceleration_x, &linear_acceleration_y, &linear_acceleration_z);
  bno.getGravityVector(&gravity_x, &gravity_y, &gravity_z);
}


float calculateAltitude(float pressure, float basePressure, float temperature) {
  float p;
  p = (basePressure / pressure);
  p = pow(p, (1 / 5.25588)) - 1.0;
  p = (p * (temperature + 273.15)) / 0.0065;
  return p;
}