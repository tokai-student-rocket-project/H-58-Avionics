#pragma once


#include <Arduino.h>


/// @brief 時間管理のクラス
class TimeManager {
public:
  uint32_t THRUST_TIME;
  uint32_t PROTECTION_SEPARATION_TIME;
  uint32_t FORCE_SEPARATION_TIME;
  uint32_t LANDING_TIME;
  uint32_t SHUTDOWN_TIME;


  /// @brief X=0をセットする
  void setZero();

  /// @brief 離昇してからの経過時間を返す
  uint32_t flightTime();

  /// @brief 指定の時間を過ぎたかを返す
  bool isElapsedTime(uint32_t time);

private:
  uint32_t _referenceTime = 0;
};