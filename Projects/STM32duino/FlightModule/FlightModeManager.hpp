#pragma once


#include <Arduino.h>
#include "Var.hpp"


/// @brief フライトモード管理のクラス
class FlightModeManager {
public:
  void changeMode(Var::FlightMode nextMode);
  Var::FlightMode currentMode();

  bool is(Var::FlightMode mode);
  bool isNot(Var::FlightMode mode);

private:
  Var::FlightMode _currentMode = Var::FlightMode::SLEEP;
};