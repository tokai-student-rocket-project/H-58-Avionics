#pragma once


#include <Arduino.h>


class AnalogVoltage {
public:
  AnalogVoltage(uint8_t pinNumber);

  float setResistance(float resistanceUpper, float resistanceLower);
  float voltage();

private:
  uint8_t _pinNumber;
  float _voltageFactor;
};