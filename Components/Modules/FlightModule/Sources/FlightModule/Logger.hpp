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


  /// @brief ログ保存を開始する
  /// @return true: 開始成功, false: 開始失敗
  bool beginLogging();

  /// @brief ログ保存を終了する
  void endLogging();

  /// @brief ログ保存の状態を返す
  /// @return true: 保存中, false: 保存中でない
  bool isLogging();


  /// @brief ログを保存する
  void log(
    uint32_t millis, uint16_t flightTime,
    uint8_t flightMode, bool cameraState, bool sn3State, bool doLogging,
    bool isFalling, bool flightPinState, bool resetPinState,
    float supplyVoltage, float batteryVoltage, float poolVoltage,
    bool isLaunchMode,
    float motorTemperature, float mcuTemperature, float current, float inputVoltage,
    float currentPosition, float currentDesiredPosition, float currentVelocity,
    uint32_t performanceMillis, float performanceTaskRate
  );


  /// @brief ロガーの使用率
  /// @return FRAM全て合わせた使用率 パーセント
  float getUsage();

private:
  bool _isLogging = false;
  uint32_t _offset = 0;

  FRAM* _fram;
};