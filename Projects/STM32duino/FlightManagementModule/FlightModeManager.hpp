#pragma once


#include <Arduino.h>


class FlightModeManager {
public:
  enum class FlightMode {
    SLEEP,
    STANDBY,
    THRUST,
    CLIMB,
    DESCENT,
    DECEL,
    PARACHUTE,
    LAND,
    SHUTDOWN
  };

  FlightModeManager();

  void changeMode(FlightMode nextMode);
  FlightMode getActiveMode();
  uint8_t getActiveModeNumber();

private:
  FlightMode _activeMode;
};