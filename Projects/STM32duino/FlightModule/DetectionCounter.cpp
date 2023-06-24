#include "DetectionCounter.hpp"


DetectionCounter::DetectionCounter(uint32_t threshold) {
  _threshold = threshold;
}


uint32_t DetectionCounter::update(bool state) {
  if (state) {
    _count++;
  }
  else {
    _count = 0;
  }

  return _count;
}


bool DetectionCounter::isDetected() {
  return _count >= _threshold;
}