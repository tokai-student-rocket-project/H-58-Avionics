#pragma once


#include <Arduino.h>
#include <MsgPacketizer.h>
#include "FRAM.hpp"


/// @brief FRAMとSDを包括したデータ保存用クラス
class Logger {
public:
  /// @brief コンストラクタ
  /// @param csFram FRAMのチップセレクト
  Logger(uint32_t csFram);


  /// @brief 書き込み位置を最初に戻す 元のデータは上書きされるので注意
  void reset();

  /// @brief 全てのデータをシリアルに出力する ブロッキング処理で時間がかかる
  void dump();

  /// @brief 全てに0を書き込み初期化する ブロッキング処理で時間がかかる
  void clear();

  /// @brief ログを保存する
  void log(
    uint32_t millis, uint32_t flightTime,
    uint8_t flightMode, bool cameraState, bool sn3State, bool doLogging,
    bool isFalling, bool flightPinState, bool resetPinState,
    float supplyVoltage, float batteryVoltage, float poolVoltage
  );

private:
  uint32_t _offset = 0;

  FRAM* _fram;
};