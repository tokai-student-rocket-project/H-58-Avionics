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
};