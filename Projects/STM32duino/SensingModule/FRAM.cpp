#include "FRAM.hpp"


FRAM::FRAM(uint32_t cs) {
  _setting = SPISettings(20'000'000, MSBFIRST, SPI_MODE0);
  _cs = cs;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
}


void FRAM::setWriteEnable() {
  beginTransaction();

  SPI.transfer(WREN);

  endTransaction();
}


void FRAM::getStatus(uint8_t* buffer) {
  beginTransaction();

  SPI.transfer(RDSR);
  buffer[0] = SPI.transfer(RDSR);

  endTransaction();
}


void FRAM::getId(uint8_t* buffer) {
  beginTransaction();

  SPI.transfer(RDID);
  buffer[0] = SPI.transfer(RDID);
  buffer[1] = SPI.transfer(RDID);
  buffer[2] = SPI.transfer(RDID);
  buffer[3] = SPI.transfer(RDID);

  endTransaction();
}


void FRAM::beginTransaction() {
  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);
}


void FRAM::endTransaction() {
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}