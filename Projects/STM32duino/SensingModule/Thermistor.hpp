#pragma once


#include <Arduino.h>
#include "DataType.hpp"


class Thermistor {
public:
  Thermistor(uint8_t pinNumber);
  void initialize();

  void getTemperature(raw_t* temperature);

private:
  const float B = 3380.0;
  const float K = 273.15;
  const float RESISTANCE_0 = 10000.0;
  const float TEMPERATURE_0 = 25.0;

  uint8_t _pinNumber;
};