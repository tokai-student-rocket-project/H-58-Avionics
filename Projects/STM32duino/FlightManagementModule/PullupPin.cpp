#include "PullupPin.hpp"


PullupPin::PullupPin(uint8_t pinNumber) {
  _pinNumber = pinNumber;
  pinMode(pinNumber, INPUT_PULLUP);
}


void PullupPin::update() {
  if (digitalRead(_pinNumber) == HIGH) {
    ++_openCount;
    _closeCount = 0;
  }
  else {
    ++_closeCount;
    _openCount = 0;
  }

  if (_openCount >= MINIMUM_COUNT) {
    _isOpen = true;
  }

  if (_closeCount >= MINIMUM_COUNT) {
    _isOpen = false;
  }
}


bool PullupPin::isOpen() {
  return _isOpen;
}