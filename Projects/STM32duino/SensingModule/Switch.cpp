#include "Switch.hpp"


/// @brief コンストラクタ
/// @param pinNumber ピン番号
Switch::Switch(uint8_t pinNumber) {
  _pin = new PullupPin(pinNumber);
}


/// @brief 現在の状態を返す
/// @param state 指定の状態
/// @return 現在の状態が指定の状態か
bool Switch::is(Var::SwitchState state) {
  return (_pin->get() ? Var::SwitchState::CLOSE : Var::SwitchState::OPEN) == state;
}