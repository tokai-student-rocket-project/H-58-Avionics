#include "PullupPin.hpp"


PullupPin::PullupPin(uint8_t pinNumber) {
  _pinNumber = pinNumber;
  pinMode(pinNumber, INPUT_PULLUP);
}


bool PullupPin::isOpen() {
  return digitalRead(_pinNumber) == HIGH;
}