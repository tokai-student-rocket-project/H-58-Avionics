#include "TimeManager.hpp"


/// @brief X=0をセットする
void TimeManager::setZero() {
  _referenceTime = millis();
}


/// @brief 離昇してからの経過時間を返す
uint32_t TimeManager::flightTime() {
  return millis() - _referenceTime;
}


/// @brief 指定の時間を過ぎたかを返す
bool TimeManager::isElapsedTime(uint32_t time) {
  return flightTime() >= time;
}