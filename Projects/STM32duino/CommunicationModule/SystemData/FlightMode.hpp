#pragma once


#include <Arduino.h>


/// @brief フライトモード管理のクラス
class FlightMode {
public:
  enum class Mode : uint8_t {
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

  void changeMode(Mode nextMode);
  Mode currentMode();

  bool is(Mode mode);
  bool isNot(Mode mode);

private:
  Mode _currentMode = Mode::SLEEP;
};