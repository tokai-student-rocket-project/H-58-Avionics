#pragma once


#include <Arduino.h>
#include <MsgPacketizer.h>
#include "FRAM.hpp"


/// @brief データ送信用クラス
class Sender {
public:
  /// @brief コンストラクタ
  /// @param csFram0 1つ目のFRAMのチップセレクト
  /// @param csFram1 2つ目のFRAMのチップセレクト
  Sender(uint32_t csFram0, uint32_t csFram1);


  /// @brief 読み出し位置を最初に戻す
  void reset();


  /// @brief ログを送信する
  /// @return 読み出したサイズ
  uint32_t send();

  uint32_t getOffset();

private:
  uint32_t _offset = 0;

  FRAM* _fram0;
  FRAM* _fram1;
};