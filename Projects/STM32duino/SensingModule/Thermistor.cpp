#include "Thermistor.hpp"


Thermistor::Thermistor(uint8_t pinNumber) {
  _pinNumber = pinNumber;
}


void Thermistor::initialize() {
  analogReadResolution(12);
  pinMode(_pinNumber, INPUT_ANALOG);
}


void Thermistor::getTemperature(float* temperature) {
  float out = analogRead(_pinNumber);
  float resistance = RESISTANCE_0 * out / (4096.0 - out);
  float temperatureK = (B * (TEMPERATURE_0 + K)) / (((TEMPERATURE_0 + K) * log(resistance / RESISTANCE_0)) + B);
  *temperature = temperatureK - K;
}