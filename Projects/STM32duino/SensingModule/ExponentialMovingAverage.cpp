// TODO コメント追加


#include "ExponentialMovingAverage.hpp"


ExponentialMovingAverage::ExponentialMovingAverage(float sensitivity) {
  _sensitivity = sensitivity;
}


void ExponentialMovingAverage::clear() {
  _previousAverage = 0;
}


void ExponentialMovingAverage::update(float value) {
  if (_previousAverage == 0) {
    _previousAverage = value;
  }

  _previousAverage = _previousAverage + _sensitivity * (value - _previousAverage);
}


float ExponentialMovingAverage::getAverage() {
  return _previousAverage;
}