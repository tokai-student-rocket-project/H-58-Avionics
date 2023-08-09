#pragma once


#include <Arduino.h>


/// @brief 列挙型まとめ
class Var {
public:
  enum class FlightMode : uint8_t {
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

  enum class Axis : uint8_t {
    X,
    Y,
    Z
  };

  enum class State : uint8_t {
    OFF,
    ON
  };

  enum class SwitchState : uint8_t {
    CLOSE,
    OPEN
  };
};