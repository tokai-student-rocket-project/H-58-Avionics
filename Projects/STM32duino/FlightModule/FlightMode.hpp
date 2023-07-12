#pragma once


#include <Arduino.h>


/// @brief フライトモード管理のスーパークラス
class FlightMode {
public:
  // TODO フライトモードを引数にとる別ライブラリはこのライブラリをインクルードする
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

  // TODO is(Mode)とかbetween(Mode, Mode)みたいな関数にする
  bool isNotSleep();
  bool isClimb();

private:
  Mode _currentMode;
};