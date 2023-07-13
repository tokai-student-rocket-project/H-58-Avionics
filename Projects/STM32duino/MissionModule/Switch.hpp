#pragma once


#include <Arduino.h>
#include "PullupPin.hpp"
#include "Var.hpp"


/// @brief スイッチの抽象化クラス
class Switch {
public:
  /// @brief コンストラクタ
  /// @param pinNumber ピン番号
  Switch(uint8_t pinNumber);


  /// @brief 現在の状態を返す
  /// @param state 指定の状態
  /// @return 現在の状態が指定の状態か
  bool is(Var::SwitchState state);

private:
  PullupPin* _pin;
};