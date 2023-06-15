#pragma once


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>


class BNO055 {
public:
  bool begin();

  String getMode();
  String getSystemStatus();
  String getSelfTestResult();
  String getSystemError();
  bool isFullyCalibrated();

  void getAcceleration(float* x, float* y, float* z);
  void getMagnetometer(float* x, float* y, float* z);
  void getGyroscope(float* x, float* y, float* z);
  void getGravityVector(float* x, float* y, float* z);
  void getLinearAcceleration(float* x, float* y, float* z);
  void getOrientation(float* x, float* y, float* z);

private:
  Adafruit_BNO055 _bno;
};