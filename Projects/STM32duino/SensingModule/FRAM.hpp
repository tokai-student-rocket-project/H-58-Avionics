#pragma once


#include <Arduino.h>
#include <SPI.h>


class FRAM {
public:
  FRAM(uint32_t cs);

  void setWriteEnable();
  void getStatus(uint8_t* buffer);
  void getId(uint8_t* buffer);

private:
  typedef enum {
    WREN = 0b0000'0110,
    RDSR = 0b0000'0101,
    RDID = 0b1001'1111
  } ope_code_t;

  SPISettings _setting;
  uint32_t _cs;

  void beginTransaction();
  void endTransaction();
};