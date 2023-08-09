#include "Thermistor.hpp"


/// @brief コンストラクタ
/// @param pinNumber アナログのピン番号
Thermistor::Thermistor(uint8_t pinNumber) {
  _pinNumber = pinNumber;
}


/// @brief ピンを初期化する
void Thermistor::initialize() {
  analogReadResolution(12);
  pinMode(_pinNumber, INPUT_ANALOG);
}


/// @brief 現在の測定温度を返す
/// @param temperature 温度のポインタ [degC]
void Thermistor::getTemperature(float* temperature) {
  float out = analogRead(_pinNumber);
  float resistance = RESISTANCE_0 * out / (4096.0 - out);

  float temperature_bar = THERM_A + THERM_B * log(resistance) + THERM_C * pow(log(resistance), 3);
  *temperature = 1.0 / temperature_bar - 273.15;
}