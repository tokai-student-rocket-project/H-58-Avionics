#pragma once


#include <Arduino.h>
#include <TaskManager.h>
#include "OutputPin.hpp"


/// @brief OUTPUT設定のピンを点滅させるためのクラス
class Blinker {
public:
  /// @brief コンストラクタ
  /// @param pinNumber ピン番号
  /// @param identify 内部でタスクを処理するための一意な名前
  Blinker(uint8_t pinNumber, String identify);


  /// @brief 点滅を開始する
  /// @param fps 1秒間に何回点滅するか
  void startBlink(float fps);

  /// @brief 点滅を終了する
  void stopBlink();


  /// @brief 点滅とは関係なくONにしたいとき用
  void on();

  /// @brief 点滅とは関係なくOFFにしたいとき用
  void off();

private:
  OutputPin* _pin;
  String _identify;
};