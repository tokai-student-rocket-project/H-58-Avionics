#include "Sd.hpp"


Sd::Sd(uint32_t cs) {
  _cs = cs;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
}


void Sd::beginLogging(String fileName) {
  _isRunning = SD.begin(_cs);
  if (_isRunning) {
    _logFile = SD.open(fileName, FILE_WRITE);
  }
}


void Sd::endLogging() {
  if (_logFile) {
    _logFile.close();
  }

  SD.end();
  _isRunning = false;
}
