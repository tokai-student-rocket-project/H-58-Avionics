#pragma once


#include <Arduino.h>
#include "OutputPin.hpp"
#include "Var.hpp"


/// @brief LEDの抽象化クラス
class Camera {
public:
  /// @brief コンストラクタ
  /// @param pinNumber ピン番号
  Camera(uint8_t pinNumber);


  /// @brief LEDを点灯する
  void on();

  /// @brief LEDを消灯する
  void off();

  /// @brief 出力を設定する
  /// @param state 出力
  void set(Var::State state);

  /// @brief 現在の状態を返す
  /// @return 現在の状態
  Var::State get();

  /// @brief 出力を入れ替える
  void toggle();

private:
  OutputPin* _pin;
};