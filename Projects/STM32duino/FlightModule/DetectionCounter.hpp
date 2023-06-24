#pragma once


#include <Arduino.h>


class DetectionCounter {
private:
  uint32_t _threshold;
  uint32_t _count;

public:
  DetectionCounter(uint32_t threshold);

  uint32_t update(bool state);
  bool isDetected();
};