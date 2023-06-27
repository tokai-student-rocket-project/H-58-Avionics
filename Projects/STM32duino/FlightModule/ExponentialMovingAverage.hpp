#pragma once


#include <Arduino.h>


class ExponentialMovingAverage {
private:
  float _sensitivity;
  float _previousAverage;

public:
  ExponentialMovingAverage(float sensitivity);

  void clear();
  void update(float value);
  float getAverage();
};