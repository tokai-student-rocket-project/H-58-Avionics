#include "FlightModeManager.hpp"


FlightModeManager::FlightModeManager() {
  changeMode(FlightMode::SLEEP);
}


void FlightModeManager::changeMode(FlightMode nextMode) {
  _activeMode = nextMode;
}


FlightModeManager::FlightMode FlightModeManager::getActiveMode() {
  return _activeMode;
}


uint8_t FlightModeManager::getActiveModeNumber() {
  return static_cast<uint8_t>(_activeMode);
}