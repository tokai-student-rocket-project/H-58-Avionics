#pragma once


#include <Arduino.h>
#include "ExponentialMovingAverage.hpp"


class Trajectory {
private:
  ExponentialMovingAverage* _altitudeAverageStrong;
  ExponentialMovingAverage* _altitudeAverageWeak;

  float _referencePressure = 1013.25;

public:
  Trajectory(float sensitivityStrong, float sensitivityWeak);

  void setReferencePressure(float referencePressure);
  float updateAltitude(float pressure, float temperature);

  float climbIndex();
  bool isFalling();
};