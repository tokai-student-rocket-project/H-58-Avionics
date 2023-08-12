#pragma once


#include <Arduino.h>
#include <Wire.h>
#include <SparkFunBME280.h>


/// @brief BME280の抽象化クラス
class BME {
public:
  /// @brief センサを初期化して起動する
  void begin();


  /// @brief 現在の気圧を返す
  /// @param pressure_hPa 気圧のポインタ [hPa]
  void getPressure(float* pressure_hPa);

  /// @brief 現在の気温を返す
  /// @param temperature_degC 気温のポインタ [degC]
  void getTemperature(float* temperature_degC);

private:
  BME280 _bme280;
};
