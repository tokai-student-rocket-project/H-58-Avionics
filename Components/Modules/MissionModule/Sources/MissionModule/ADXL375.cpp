#include "ADXL375.hpp"


ADXL375::ADXL375(uint32_t cs) {
  _cs = cs;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
}


void ADXL375::begin() {
  // TODO 高ODR LSB問題
  // BW_RATE <- 3200Hz
  write(0x2C, 0b00001111);

  // POWER_CTL <- Measure
  write(0x2D, 0b00001000);
}


void ADXL375::getAcceleration(float* x, float* y, float* z) {
  *x = (float)read16(0x32) * 0.049 * 9.80665;
  *y = (float)read16(0x34) * 0.049 * 9.80665;
  *z = (float)read16(0x36) * 0.049 * 9.80665;
}


int16_t ADXL375::read16(uint8_t address) {
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);

  SPI.transfer(address | 0b11000000);
  uint8_t data0 = SPI.transfer(0xFF);
  uint8_t data1 = SPI.transfer(0xFF);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  return uint16_t(data1) << 8 | uint16_t(data0);
}


void ADXL375::write(uint8_t address, uint8_t data) {
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);

  SPI.transfer(address);
  SPI.transfer(data);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}