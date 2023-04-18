#include <Wire.h>
#include <TaskManager.h>
#include "DataType.hpp"
// #include "CANBUS.hpp"
#include "BNO055.hpp"
#include "LPS33HW.hpp"
#include "Thermistor.hpp"


// CANBUS canbus;

BNO055 bno055(&Wire, 0x28);
LPS33HW lps33hw(&Wire, 0x5C);
Thermistor thermistor(PA_2);

raw_t acceleration_x, acceleration_y, acceleration_z;
raw_t magnetometer_x, magnetometer_y, magnetometer_z;
raw_t gyroscope_x, gyroscope_y, gyroscope_z;
raw_t quaternion_w, quaternion_x, quaternion_y, quaternion_z;
raw_t gravity_x, gravity_y, gravity_z;
raw_t linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
raw_t euler_heading, euler_roll, euler_pitch;

raw_t pressure;
raw_t temperature;
raw_t altitude;

raw_t basePressure = raw_t::raw(1013.25);

void setup() {
  analogReadResolution(12);

  Serial.begin(115200);

  Wire.setSDA(PB_7);
  Wire.setSCL(PB_6);
  Wire.begin();
  Wire.setClock(400000);

  bno055.initialize();
  lps33hw.initialize();
  thermistor.initialize();

  // canbus.initialize();

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
  bno055.getMagnetometer(&magnetometer_x, &magnetometer_y, &magnetometer_z);
}


void task50Hz() {
  lps33hw.getPressure(&pressure);
  altitude = calculateAltitude(pressure, basePressure, temperature);

  Serial.println(temperature.toFloat(4096.0));
}


void task100Hz() {
  bno055.getAcceleration(&acceleration_x, &acceleration_y, &acceleration_z);
  bno055.getGyroscope(&gyroscope_x, &gyroscope_y, &gyroscope_z);
  bno055.getQuaternion(&quaternion_w, &quaternion_x, &quaternion_y, &quaternion_z);
  bno055.getGravityVector(&gravity_x, &gravity_y, &gravity_z);
  bno055.getLinearAcceleration(&linear_acceleration_x, &linear_acceleration_y, &linear_acceleration_z);
  bno055.getEuler(&euler_heading, &euler_roll, &euler_pitch);
}


raw_t calculateAltitude(raw_t pressure, raw_t basePressure, raw_t temperature) {
  float p;
  p = (basePressure.toFloat(4096.0) / pressure.toFloat(4096.0));
  p = pow(p, (1 / 5.25588)) - 1.0;
  p = (p * (temperature.toFloat(4096.0) + 273.15)) / 0.0065;
  return raw_t::raw(p);
}