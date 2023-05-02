#pragma once


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL375.h>


class ADXL375 {
public:
  void begin();

  void setOffsets();
  void getAcceleration(float* x, float* y, float* z);

private:
  Adafruit_ADXL375 _adxl = Adafruit_ADXL375(-1, &Wire);
};