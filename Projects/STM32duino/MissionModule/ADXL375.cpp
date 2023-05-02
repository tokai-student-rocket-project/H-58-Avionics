#include "ADXL375.hpp"


void ADXL375::begin() {
  _adxl.begin();
  _adxl.setDataRate(ADXL3XX_DATARATE_1600_HZ);
}


void ADXL375::setOffsets() {
  _adxl.setTrimOffsets(0, 0, 0);

  int16_t x = _adxl.getX();
  int16_t y = _adxl.getY();
  int16_t z = _adxl.getZ();

  _adxl.setTrimOffsets(-(x + 2) / 4, -(y + 2) / 4, -(z - 20 + 2) / 4);
}


void ADXL375::getAcceleration(float* x, float* y, float* z) {
  sensors_event_t event;
  _adxl.getEvent(&event);
  *x = event.acceleration.x;
  *y = event.acceleration.y;
  *z = event.acceleration.z;
}