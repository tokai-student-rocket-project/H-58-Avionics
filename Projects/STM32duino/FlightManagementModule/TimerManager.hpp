#pragma once


#include <Arduino.h>


class TimerManager {
public:
  uint32_t ThrustTime;
  uint32_t ApogeeTime;
  uint32_t FirstSeparationTime;
  uint32_t SecondSeparationTime;
  uint32_t LandTime;
  uint32_t ShutdownTime;

  TimerManager();

  void setReferenceTime();
  bool isElapsedTime(uint32_t time);

private:
  uint32_t _referenceTime;
};