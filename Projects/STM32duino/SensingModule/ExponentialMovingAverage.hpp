#pragma once


#include <Arduino.h>


class ExponentialMovingAverage {
private:
  float _sensitivity = 0;
  float _previousAverage = 0;

public:
  ExponentialMovingAverage(float sensitivity);

  void clear();
  void update(float value);
  float getAverage();
};