#pragma once


#include <Arduino.h>
#include <TaskManager.h>
#include "OutputPin.hpp"


/// @brief 不知火の抽象化クラス
class Shiranui {
public:
  /// @brief コンストラクタ
  /// @param pinNumber ピン番号
  /// @param identify 認識用の一意な名前
  Shiranui(uint8_t pinNumber, String identify);


  /// @brief 分離信号を出力する
  void separate();

  /// @brief 現在の出力を返す
  /// @return 現在の出力
  bool get();

private:
  OutputPin* _pin;
  String _identify;
};