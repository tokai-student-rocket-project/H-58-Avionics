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


  // TODO 衛星数

  /// @brief 受信した緯度を返す
  /// @return 緯度 [deg]
  float getLatitude();

  /// @brief 受信した経度を返す
  /// @return 経度 [deg]
  float getLongitude();

private:
  SFE_UBLOX_GNSS _gnss;
};