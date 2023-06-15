#include "BME280.hpp"


bool BME::begin() {
  _bme280.setI2CAddress(0x76);
  bool isSucceeded = _bme280.beginI2C();

  return isSucceeded;
}


String BME::getMode() {
  switch (_bme280.getMode()) {
  case 0:
    return "Sleep";
    break;
  case 3:
    return "Normal";
    break;
  }
}


void BME::getPressure(float* pressure) {
  _bme280.readTempC();
  *pressure = _bme280.readFloatPressure() / 100.0;
}