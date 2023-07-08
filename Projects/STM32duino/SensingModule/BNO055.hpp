// TODO コメント追加


#pragma once


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>


class BNO055 {
public:
  void begin();

  void getAcceleration(float* x, float* y, float* z);
  void getMagnetometer(float* x, float* y, float* z);
  void getGyroscope(float* x, float* y, float* z);
  void getGravityVector(float* x, float* y, float* z);
  void getLinearAcceleration(float* x, float* y, float* z);
  void getOrientation(float* x, float* y, float* z);

private:
  Adafruit_BNO055 _bno;
};