#pragma once


#include <Arduino.h>


/// @brief OUTPUT設定のピンの抽象化クラス
class OutputPin {
public:
  /// @brief コンストラクタ
  /// @param pinNumber ピン番号
  OutputPin(uint8_t pinNumber);


  /// @brief 出力をHIGHにする
  void on();

  /// @brief 出力をLOWにする
  void off();

  /// @brief ブール値で出力を設定する
  /// @param isOn true: HIGH, false: Low
  void set(bool isOn);

  /// @brief 現在の出力を返す
  /// @return 現在の出力
  bool get();

  /// @brief 出力を入れ替える
  void toggle();

private:
  uint8_t _pinNumber;
};