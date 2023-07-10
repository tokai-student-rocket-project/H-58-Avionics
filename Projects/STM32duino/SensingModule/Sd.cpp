#include "Sd.hpp"


Sd::Sd(uint32_t cs) {
  _cs = cs;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
}


bool Sd::begin() {
  bool isSucceeded = SD.begin(_cs);

  _isRunning = isSucceeded;
  return isSucceeded;
}


void Sd::end() {
  SD.end();
  _isRunning = false;
}


void Sd::beginLogging(String fileName) {
  _logFile = SD.open(fileName, FILE_WRITE);
}


void Sd::endLogging() {
  _logFile.close();
}


bool Sd::isRunning() {
  return _isRunning;
}