#pragma once


#include <Arduino.h>
#include <TaskManager.h>
#include "OutputPin.hpp"


/// @brief ブザーの抽象化クラス
class Buzzer {
public:
  /// @brief コンストラクタ
  /// @param pinNumber ピン番号
  /// @param identify 認識用の一意な名前
  Buzzer(uint8_t pinNumber, String identify);


  /// @brief 短音1回を鳴らす
  void beepOnce();

  /// @brief 短音2回を鳴らす
  void beepTwice();

  /// @brief 超音1回を鳴らす
  void beepLongOnce();


  /// @brief ほろびのうたを鳴らす ブロッキング処理なので注意
  void electricalParade();

  // 以下ほろびのうた用
  /// @brief 2分音符
  void beep2();

  /// @brief 4分音符
  void beep4();

  /// @brief 8分音符
  void beep8();

  /// @brief 4分休符
  void sleep4();

  /// @brief 8分休符
  void sleep8();

private:
  OutputPin* _pin;
  String _identify;
};