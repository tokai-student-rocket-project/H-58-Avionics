#pragma once


#include <Arduino.h>
#include <SPI.h>
#include <SD.h>


class Sd {
public:
  Sd(uint32_t cs);

  bool begin();
  void end();

  void beginLogging(String fileName);
  void endLogging();

  bool isRunning();

private:
  uint32_t _cs;
  bool _isRunning = false;

  File _logFile;
};