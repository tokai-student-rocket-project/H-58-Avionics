// TODO コメント追加


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

  float update(float pressure, float temperature);
  float update(float altitude);

  float climbIndex();
  bool isFalling();
};