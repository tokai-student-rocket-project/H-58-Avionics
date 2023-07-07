#pragma once


#include <Arduino.h>
#include "ExponentialMovingAverage.hpp"


class Trajectory {
private:
  ExponentialMovingAverage* _altitudeAverageStrong;
  ExponentialMovingAverage* _altitudeAverageWeak;

public:
  Trajectory(float sensitivityStrong, float sensitivityWeak);

  void update(float altitude);
  float climbIndex();
  bool isDescending();
};