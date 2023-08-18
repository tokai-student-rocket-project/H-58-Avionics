#pragma once


#include <Arduino.h>
#include <MsgPacketizer.h>
#include "FRAM.hpp"


/// @brief FRAMとSDを包括したデータ保存用クラス
class Logger {
public:
  /// @brief コンストラクタ
  /// @param csFram0 1つ目のFRAMのチップセレクト
  /// @param csFram1 2つ目のFRAMのチップセレクト
  Logger(uint32_t csFram0, uint32_t csFram1);


  /// @brief 書き込み位置を最初に戻す 元のデータは上書きされるので注意
  void reset();

  /// @brief 全てのデータをシリアルに出力する ブロッキング処理で時間がかかる
  void dump();

  /// @brief 全てに0を書き込み初期化する ブロッキング処理で時間がかかる
  void clear();


  /// @brief ログを保存する
  /// @return 書き込んだサイズ
  uint32_t log(
    uint32_t micros, uint8_t flightMode,
    uint8_t x0, uint8_t x1,
    uint8_t y0, uint8_t y1,
    uint8_t z0, uint8_t z1
  );

  uint32_t getOffset();

  /// @brief ロガーの使用率
  /// @return FRAM全て合わせた使用率 パーセント
  float getUsage();

private:
  uint32_t _offset = 0;

  FRAM* _fram0;
  FRAM* _fram1;
};