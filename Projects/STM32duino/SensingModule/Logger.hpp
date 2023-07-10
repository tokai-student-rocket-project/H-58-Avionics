#pragma once


#include <Arduino.h>
#include <MsgPacketizer.h>
#include "FRAM.hpp"
#include "Sd.hpp"


/// @brief FRAMとSDを包括したデータ保存用クラス
class Logger {
public:
  /// @brief コンストラクタ
  /// @param csFram0 1つ目のFRAMのチップセレクト
  /// @param csFram1 2つ目のFRAMのチップセレクト
  /// @param csSd SDのチップセレクト
  Logger(uint32_t csFram0, uint32_t csFram1, uint32_t csSd);


  /// @brief ログ保存を開始する
  /// @return true: 開始成功, false: 開始失敗
  bool beginLogging();

  /// @brief ログ保存を終了する
  void endLogging();


  /// @brief 書き込み位置を最初に戻す 元のデータは上書きされるので注意
  void reset();

  /// @brief 全てのデータをシリアルに出力する ブロッキング処理で時間がかかる
  void dump();

  /// @brief 全てに0を書き込み初期化する ブロッキング処理で時間がかかる
  void clear();

  /// @brief ログを保存する
  void log(
    uint32_t millis,
    float outsideTemperature, float pressure, float altitude, float climbIndex, bool isFalling,
    float acceleration_x, float acceleration_y, float acceleration_z,
    float gyroscope_x, float gyroscope_y, float gyroscope_z,
    float magnetometer_x, float magnetometer_y, float magnetometer_z,
    float orientation_x, float orientation_y, float orientation_z,
    float linear_acceleration_x, float linear_acceleration_y, float linear_acceleration_z,
    float gravity_x, float gravity_y, float gravity_z
  );

private:
  uint32_t _offset = 0;

  FRAM* _fram0;
  FRAM* _fram1;

  Sd* _sd;
};