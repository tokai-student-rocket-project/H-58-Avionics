#pragma once


#include <Arduino.h>


/// @brief 温度測定用サーミスタの抽象化クラス
class Thermistor {
public:
  /// @brief コンストラクタ
  /// @param pinNumber アナログのピン番号
  Thermistor(uint8_t pinNumber);

  /// @brief ピンを初期化する
  void initialize();


  /// @brief 現在の測定温度を返す
  /// @param temperature 温度のポインタ [degC]
  void getTemperature(float* temperature);

private:
  /// @brief サーミスタの抵抗値 データシート参照
  const float RESISTANCE_0 = 10000.0;

  /// @brief スタインハート式で求めた定数たち サーミスタごとに校正する
  const float THERM_A = 2.7743616E-4;
  const float THERM_B = 3.67106347E-4;
  const float THERM_C = -3.5569334E-7;

  uint8_t _pinNumber;
};