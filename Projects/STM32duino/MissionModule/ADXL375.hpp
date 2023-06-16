#pragma once


#include <Arduino.h>
#include <Wire.h>


class ADXL375 {
public:
  void begin();

  void getAcceleration(float* x, float* y, float* z);
};