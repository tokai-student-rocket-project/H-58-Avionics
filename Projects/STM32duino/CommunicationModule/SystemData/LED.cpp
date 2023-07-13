#include "LED.hpp"


/// @brief コンストラクタ
/// @param pinNumber ピン番号
LED::LED(uint8_t pinNumber) {
  _pin = new OutputPin(pinNumber);
}


/// @brief LEDを点灯する
void LED::on() {
  _pin->high();
}


/// @brief LEDを消灯する
void LED::off() {
  _pin->low();
}


/// @brief 出力を設定する
/// @param state 出力
void LED::set(Var::State state) {
  _pin->set(state == Var::State::ON);
}


/// @brief 現在の状態を返す
/// @return 現在の状態
Var::State LED::get() {
  return _pin->get() ? Var::State::ON : Var::State::OFF;
}


/// @brief 出力を入れ替える
void LED::toggle() {
  _pin->toggle();
}