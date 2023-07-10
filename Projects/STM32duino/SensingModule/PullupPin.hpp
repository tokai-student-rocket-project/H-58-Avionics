#pragma once


#include <Arduino.h>


/// @brief PULLUP設定のピンの抽象化クラス
class PullupPin {
public:
  /// @brief コンストラクタ
  /// @param pinNumber ピン番号
  PullupPin(uint8_t pinNumber);

  /// @brief 現在の状態を返す
  /// @return true: 回路切断, false: 回路接続
  bool isOpen();

private:
  uint8_t _pinNumber;
};