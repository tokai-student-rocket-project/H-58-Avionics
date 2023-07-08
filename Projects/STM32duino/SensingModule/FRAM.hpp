#pragma once


#include <Arduino.h>
#include <SPI.h>


class FRAM {
public:
  static const uint32_t LENGTH = 262144;

  FRAM(uint32_t cs);

  void setWriteEnable();
  void getStatus(uint8_t* buffer);
  void getId(uint8_t* buffer);

  uint8_t read(uint32_t address);
  void write(uint32_t address, uint8_t data);
  void write(uint32_t address, const uint8_t* data, uint32_t size);

  void clear();
  void dump();

private:
  typedef enum {
    WREN = 0b0000'0110,
    RDSR = 0b0000'0101,
    READ = 0b0000'0011,
    WRITE = 0b0000'0010,
    RDID = 0b1001'1111
  } ope_code_t;

  SPISettings _setting;
  uint32_t _cs;
};