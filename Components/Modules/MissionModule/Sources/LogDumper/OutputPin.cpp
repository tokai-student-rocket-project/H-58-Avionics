#include "OutputPin.hpp"


/// @brief コンストラクタ
/// @param pinNumber ピン番号
OutputPin::OutputPin(uint8_t pinNumber) {
  _pinNumber = pinNumber;
  pinMode(pinNumber, OUTPUT);
}


/// @brief 出力をHIGHにする
void OutputPin::high() {
  digitalWrite(_pinNumber, HIGH);
}


/// @brief 出力をLOWにする
void OutputPin::low() {
  digitalWrite(_pinNumber, LOW);
}


/// @brief ブール値で出力を設定する
/// @param isHigh true: HIGH, false: Low
void OutputPin::set(bool isHigh) {
  digitalWrite(_pinNumber, isHigh ? HIGH : LOW);
}


/// @brief 現在の出力を返す
/// @return 現在の出力
bool OutputPin::get() {
  return digitalRead(_pinNumber) == HIGH;
}


/// @brief 出力を入れ替える
void OutputPin::toggle() {
  // 一行で出力を入れ替えられる神
  digitalWrite(_pinNumber, !digitalRead(_pinNumber));
}