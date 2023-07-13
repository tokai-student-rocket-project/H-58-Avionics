#pragma once


#include <Arduino.h>


/// @brief PULLUP設定のピンの抽象化クラス
class PullupPin {
public:
  /// @brief コンストラクタ
  /// @param pinNumber ピン番号
  PullupPin(uint8_t pinNumber);

  /// @brief 現在の状態を返す
  /// @return true: HIGH, false: Low
  bool get();

private:
  uint8_t _pinNumber;
};