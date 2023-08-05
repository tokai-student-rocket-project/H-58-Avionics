#include "GNSS.hpp"


/// @brief 開始する
void GNSS::begin() {
  Wire.begin();

  _gnss.begin();
  _gnss.setI2COutput(COM_TYPE_UBX);
  _gnss.setNavigationFrequency(10);
  _gnss.setAutoPVT(true);
}


/// @brief GNSS情報を受信しているかを返す
/// @return true: 受信している, false: 受信していない
bool GNSS::available() {
  if (!_gnss.getPVT() || _gnss.getInvalidLlh()) {
    return false;
  }

  // 受信しているなら保存しておく
  int32_t latitude = _gnss.getLatitude();
  int32_t longitude = _gnss.getLongitude();

  return latitude != 0 && longitude != 0;
}


/// @brief 受信した緯度を返す
/// @return 緯度 [deg]
float GNSS::getLatitude() {
  return (float)_gnss.getLatitude() / 10000000.0;
}


/// @brief 受信した経度を返す
/// @return 経度 [deg]
float GNSS::getLongitude() {
  return (float)_gnss.getLongitude() / 10000000.0;
}


/// @brief 衛星数を返す
/// @return 衛星数
uint8_t GNSS::getSatelliteCount() {
  return _gnss.getSIV();
}


/// @brief 固定されたかを返す
bool GNSS::isFixed() {
  return _gnss.getGnssFixOk();
}


/// @brief 位置の特定方法を返す
/// @return 0=no, 3=3D, 4=GNSS+Deadreckoning
uint8_t GNSS::getFixType() {
  return _gnss.getFixType();
}


/// @brief 受信した高度を返す
/// @return 高度 [m]
float GNSS::getAltitude() {
  return _gnss.getAltitude() / 1000.0;
}


/// @brief 受信した対地速度を返す
/// @return 速度 [m/s]
float GNSS::getSpeed() {
  return _gnss.getGroundSpeed() / 1000.0;
}