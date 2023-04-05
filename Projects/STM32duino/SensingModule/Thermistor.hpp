#ifndef THERMISTOR_HPP_
#define THERMISTOR_HPP_


#include <Arduino.h>


class Thermistor {
public:
  Thermistor(uint8_t pinNumber);
  void initialize();

  void getTemperature(double* temperature);

private:
  const double B = 3380.0;
  const double K = 273.15;
  const double RESISTANCE_0 = 10000.0;
  const double TEMPERATURE_0 = 25.0;

  uint8_t _pinNumber;
};


#endif