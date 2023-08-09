#pragma once


#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>


/// @brief Arduino MKR GPS Shield
class GNSS {
public:
  /// @brief 開始する
  void begin();

  /// @brief GNSS情報を受信しているかを返す
  /// @return true: 受信している, false: 受信していない
  bool available();

  /// @brief 受信した緯度を返す
  /// @return 緯度 [deg]
  float getLatitude();

  /// @brief 受信した経度を返す
  /// @return 経度 [deg]
  float getLongitude();

  /// @brief 衛星数を返す
  /// @return 衛星数
  uint8_t getSatelliteCount();

  /// @brief 位置が特定されたかを返す
  bool isFixed();

  /// @brief 位置の特定方法を返す
  /// @return 0=no, 3=3D, 4=GNSS+Deadreckoning
  uint8_t getFixType();

  /// @brief 受信した高度を返す
  /// @return 高度 [m]
  float getAltitude();

  /// @brief 受信した対地速度を返す
  /// @return 速度 [m/s]
  float getSpeed();

private:
  SFE_UBLOX_GNSS _gnss;
};