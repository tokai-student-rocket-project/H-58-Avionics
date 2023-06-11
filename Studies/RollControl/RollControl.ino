#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <PID_v1.h>
#include <TaskManager.h>


Adafruit_BNO055 bno;

double gyro, roll;
double control;

double target = 0;

double kp = 0.2, ki = 0, kd = 0;

PID gyroPID(&gyro, &control, &target, kp, ki, kd, DIRECT);
PID rollPID(&roll, &control, &target, kp, ki, kd, DIRECT);


void setup() {
  Serial.begin(115200);

  Wire.begin(400000);

  bno.begin();
  bno.setExtCrystalUse(true);

  gyroPID.SetMode(AUTOMATIC);
  gyroPID.SetOutputLimits(-20, 20);
  gyroPID.SetSampleTime(10);

  rollPID.SetMode(AUTOMATIC);
  rollPID.SetOutputLimits(-20, 20);
  rollPID.SetSampleTime(10);

  Tasks.add(routine)->startIntervalMsec(10);
}


void loop() {
  Tasks.update();
}


void routine() {
  // gyro = readGyro();
  // gyroPID.Compute();
  // Serial.print("gyro:");
  // Serial.print(gyro);

  roll = readRoll() - 180.0;
  rollPID.Compute();
  Serial.print("roll:");
  Serial.print(roll);

  Serial.print(",control:");
  Serial.print(control);
  Serial.print(",target:");
  Serial.println(target);
}


double readValue(uint8_t address) {
  Wire.beginTransmission(0x28);
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(0x28, 2);

  while (Wire.available() < 2);

  int16_t value = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);
  return (double)value / 16.0;
}


double readGyro() {
  // read GYRO_DATA_Z_LSB_ADDR
  return readValue(0x18);
}


double readRoll() {
  // read EULER_H_LSB_ADDR
  return readValue(0x1A);
}