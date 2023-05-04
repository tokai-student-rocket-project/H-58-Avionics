#pragma once


#include <Arduino.h>
#include <SPI.h>
#include <SD.h>


class Sd {
public:
  Sd(uint32_t cs);

  bool begin();
  void end();

  bool isRunning();

private:
  uint32_t _cs;
  bool _isRunning = false;
};