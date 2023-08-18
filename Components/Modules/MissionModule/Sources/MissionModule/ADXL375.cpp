#include "ADXL375.hpp"


ADXL375::ADXL375(uint32_t cs) {
  _cs = cs;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
}


void ADXL375::begin() {
  // BW_RATE <- 3200Hz
  write(0x2C, 0b00001111);

  // POWER_CTL <- Measure
  write(0x2D, 0b00001000);
}


void ADXL375::getAcceleration(uint8_t* x0, uint8_t* x1, uint8_t* y0, uint8_t* y1, uint8_t* z0, uint8_t* z1) {
  *x0 = read8(0x32);
  *x1 = read8(0x33);
  *y0 = read8(0x34);
  *y1 = read8(0x35);
  *z0 = read8(0x36);
  *z1 = read8(0x37);
}


uint8_t ADXL375::read8(uint8_t address) {
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);

  SPI.transfer(address | 0b11000000);
  uint8_t data = SPI.transfer(0xFF);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  return data;
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