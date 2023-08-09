#include "PullupPin.hpp"


/// @brief コンストラクタ
/// @param pinNumber ピン番号
PullupPin::PullupPin(uint8_t pinNumber) {
  _pinNumber = pinNumber;
  pinMode(pinNumber, INPUT_PULLUP);
}


/// @brief 現在の状態を返す
/// @return true: HIGH, false: Low
bool PullupPin::get() {
  return digitalRead(_pinNumber) == HIGH;
}