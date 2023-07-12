#include "FlightMode.hpp"


void FlightMode::changeMode(Mode nextMode) {
  // フライトモードに変更がないなら何もしない
  if (_currentMode == nextMode) {
    return;
  }

  _currentMode = nextMode;
}


FlightMode::Mode FlightMode::currentMode() {
  return _currentMode;
}


bool FlightMode::isNotSleep() {
  return _currentMode != Mode::SLEEP;
}


bool FlightMode::isClimb() {
  return _currentMode == Mode::CLIMB;
}