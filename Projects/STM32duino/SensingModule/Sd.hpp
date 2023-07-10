#pragma once


#include <Arduino.h>
#include <SPI.h>
#include <SD.h>


class Sd {
public:
  Sd(uint32_t cs);

  void beginLogging(String fileName);
  void endLogging();

private:
  uint32_t _cs;
  bool _isRunning = false;

  File _logFile;
};