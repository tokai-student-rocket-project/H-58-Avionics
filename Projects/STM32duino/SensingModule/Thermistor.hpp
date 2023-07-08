// TODO コメント追加


#pragma once


#include <Arduino.h>


class Thermistor {
public:
  Thermistor(uint8_t pinNumber);
  void initialize();

  void getTemperature(float* temperature);

private:
  const float RESISTANCE_0 = 10000.0;

  const float THERM_A = 2.7743616E-4;
  const float THERM_B = 3.67106347E-4;
  const float THERM_C = -3.5569334E-7;

  uint8_t _pinNumber;
};