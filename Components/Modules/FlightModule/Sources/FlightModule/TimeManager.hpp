#pragma once


#include <Arduino.h>


/// @brief 時間管理のクラス
class TimeManager {
public:
  uint16_t THRUST_TIME;
  uint16_t PROTECTION_SEPARATION_TIME;
  uint16_t FORCE_SEPARATION_TIME;
  uint16_t DATA_SAVING_TIME;
  uint16_t LANDING_TIME;
  uint16_t SHUTDOWN_TIME;


  /// @brief X=0をセットする
  void setZero();

  /// @brief 離昇してからの経過時間を返す
  uint16_t flightTime();

  /// @brief 指定の時間を過ぎたかを返す
  bool isElapsedTime(uint16_t time);

private:
  uint32_t _referenceTime = 0;
};