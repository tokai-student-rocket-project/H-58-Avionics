#pragma once


#include <Arduino.h>
#include "ExponentialMovingAverage.hpp"


class ApogeeDetector {
private:
  ExponentialMovingAverage* _altitudeAverageStrong;
  ExponentialMovingAverage* _altitudeAverageWeak;

public:
  ApogeeDetector(float sensitivityStrong, float sensitivityWeak);

  void update(float altitude);
  bool isDetected();
};