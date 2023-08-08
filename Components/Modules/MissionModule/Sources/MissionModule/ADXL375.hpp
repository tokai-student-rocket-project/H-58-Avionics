#pragma once


#include <Arduino.h>
#include <SPI.h>


class ADXL375 {
public:
  ADXL375(uint32_t cs);

  void begin();
  void getAcceleration(float* x, float* y, float* z);

private:
  uint32_t _cs;
  SPISettings _spiSettings = SPISettings(4000000, MSBFIRST, SPI_MODE3);

  int16_t read16(uint8_t address);
  void write(uint8_t address, uint8_t data);
};