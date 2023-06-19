#include "AnalogVoltage.hpp"


AnalogVoltage::AnalogVoltage(uint8_t pinNumber) {
  _pinNumber = pinNumber;
  pinMode(_pinNumber, INPUT_ANALOG);
}


float AnalogVoltage::setResistance(float resistanceUpper, float resistanceLower) {
  _voltageFactor = (resistanceUpper + resistanceLower) / resistanceLower;
  return _voltageFactor;
}


float AnalogVoltage::voltage() {
  float voltage = (float)analogRead(_pinNumber) / 4095.0 * 3.3;
  return voltage * _voltageFactor;
}