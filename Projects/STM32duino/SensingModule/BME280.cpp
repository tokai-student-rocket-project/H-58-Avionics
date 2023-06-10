#include "BME280.hpp"


void BME::begin() {
  _bme280.setI2CAddress(0x76);
  _bme280.beginI2C();
}


void BME::getPressure(float* pressure) {
  _bme280.readTempC();
  *pressure = _bme280.readFloatPressure() / 100.0;
}