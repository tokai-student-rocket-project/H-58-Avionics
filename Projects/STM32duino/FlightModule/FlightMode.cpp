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


bool FlightMode::is(Mode mode) {
  return _currentMode == mode;
}


bool FlightMode::isNot(Mode mode) {
  return _currentMode != mode;
}