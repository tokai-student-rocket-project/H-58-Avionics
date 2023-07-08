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


uint8_t FRAM::read(uint32_t address) {
  uint8_t addressPart[3];
  memcpy(addressPart, &address, 3);

  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);

  SPI.transfer(READ);
  SPI.transfer(addressPart[2]);
  SPI.transfer(addressPart[1]);
  SPI.transfer(addressPart[0]);
  uint8_t data = SPI.transfer(0xFF);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  return data;
}


void FRAM::write(uint32_t address, uint8_t data) {
  uint8_t addressPart[3];
  memcpy(addressPart, &address, 3);

  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);

  SPI.transfer(WRITE);
  SPI.transfer(addressPart[2]);
  SPI.transfer(addressPart[1]);
  SPI.transfer(addressPart[0]);
  SPI.transfer(data);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}


void FRAM::write(uint32_t address, const uint8_t* data, uint32_t size) {
  uint8_t addressPart[3];
  memcpy(addressPart, &address, 3);

  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);

  SPI.transfer(WRITE);
  SPI.transfer(addressPart[2]);
  SPI.transfer(addressPart[1]);
  SPI.transfer(addressPart[0]);

  for (uint32_t i = 0; i < size; i++) {
    SPI.transfer(data[i]);
  }

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


void FRAM::clear() {
  for (uint32_t address = 0; address < LENGTH; address++) {
    write(address, 0x00);
  }
}


void FRAM::dump() {
  for (size_t address = 0; address < LENGTH; address++) {
    uint8_t data = read(address);

    Serial.print(data, HEX);

    if (data == 0) {
      Serial.println();
    }
    else {
      Serial.print(" ");
    }
  }
}