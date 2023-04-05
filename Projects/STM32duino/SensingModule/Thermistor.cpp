#include "Thermistor.hpp"


Thermistor::Thermistor(uint8_t pinNumber) {
  _pinNumber = pinNumber;
}


void Thermistor::initialize() {
  pinMode(_pinNumber, INPUT_ANALOG);
}


void Thermistor::getTemperature(double* temperature) {
  double out = analogRead(_pinNumber);
  double resistance = RESISTANCE_0 * out / (4096.0 - out);
  double temperatureK = (B * (TEMPERATURE_0 + K)) / (((TEMPERATURE_0 + K) * log(resistance / RESISTANCE_0)) + B);
  *temperature = temperatureK - K;
}