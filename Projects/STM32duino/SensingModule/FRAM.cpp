#include "FRAM.hpp"


FRAM::FRAM(uint32_t cs) {
  _setting = SPISettings(20'000'000, MSBFIRST, SPI_MODE0);
  _cs = cs;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
}


void FRAM::setWriteEnable() {
  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);

  SPI.transfer(WREN);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}


void FRAM::getStatus(uint8_t* buffer) {
  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);

  SPI.transfer(RDSR);
  buffer[0] = SPI.transfer(0xFF);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}


void FRAM::getId(uint8_t* buffer) {
  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);

  SPI.transfer(RDID);
  buffer[0] = SPI.transfer(0xFF);
  buffer[1] = SPI.transfer(0xFF);
  buffer[2] = SPI.transfer(0xFF);
  buffer[3] = SPI.transfer(0xFF);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}