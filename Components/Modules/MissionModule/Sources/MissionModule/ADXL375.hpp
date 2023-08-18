#pragma once


#include <Arduino.h>
#include <SPI.h>


class ADXL375 {
public:
  ADXL375(uint32_t cs);

  void begin();
  void getAcceleration(uint8_t* x0, uint8_t* x1, uint8_t* y0, uint8_t* y1, uint8_t* z0, uint8_t* z1);

private:
  uint32_t _cs;
  SPISettings _spiSettings = SPISettings(4000000, MSBFIRST, SPI_MODE3);

  uint8_t read8(uint8_t address);
  int16_t read16(uint8_t address);
  void write(uint8_t address, uint8_t data);
};