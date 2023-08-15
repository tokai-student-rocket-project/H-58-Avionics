#include "TimeManager.hpp"


/// @brief X=0をセットする
void TimeManager::setZero() {
  _referenceTime = millis();
}


/// @brief 離昇してからの経過時間を返す
uint16_t TimeManager::flightTime() {
  return (uint16_t)(millis() - _referenceTime);
}


/// @brief 指定の時間を過ぎたかを返す
bool TimeManager::isElapsedTime(uint16_t time) {
  return flightTime() >= time;
}