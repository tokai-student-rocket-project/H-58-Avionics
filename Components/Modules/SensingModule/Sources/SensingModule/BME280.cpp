#include "BME280.hpp"


/// @brief センサを初期化して起動する
void BME::begin() {
  _bme280.setI2CAddress(0x76);
  _bme280.beginI2C();
}


/// @brief 現在の気圧を返す
/// @param pressure_hPa 気圧のポインタ [hPa]
void BME::getPressure(float* pressure_hPa) {
  // BME280の仕様で気圧計測の前に気温計測を行う必要がある
  // データシートの 3.4 Measurement Flow 参照
  _bme280.readTempC();

  // readFloatPressure()は[Pa]で帰ってくるので100で割って[hPa]にする
  float pressure_Pa = _bme280.readFloatPressure();
  *pressure_hPa = pressure_Pa / 100.0;
}
