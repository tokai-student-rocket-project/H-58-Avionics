#include "FlightModeManager.hpp"


void FlightModeManager::changeMode(Var::FlightMode nextMode) {
  // フライトモードに変更がないなら何もしない
  if (_currentMode == nextMode) {
    return;
  }

  _currentMode = nextMode;
}


Var::FlightMode FlightModeManager::currentMode() {
  return _currentMode;
}


bool FlightModeManager::is(Var::FlightMode mode) {
  return _currentMode == mode;
}


bool FlightModeManager::isNot(Var::FlightMode mode) {
  return _currentMode != mode;
}