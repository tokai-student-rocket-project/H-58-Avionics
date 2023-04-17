#include "TimerManager.hpp"


TimerManager::TimerManager() {
  ThrustTime = 3000;
  ApogeeTime = 10000;
  FirstSeparationTime = 11000;
  SecondSeparationTime = 15000;
  LandTime = 25000;
  ShutdownTime = 26000;
}


void TimerManager::setReferenceTime() {
  _referenceTime = millis();
}


bool TimerManager::isElapsedTime(uint32_t time) {
  return (millis() - _referenceTime) >= time;
}