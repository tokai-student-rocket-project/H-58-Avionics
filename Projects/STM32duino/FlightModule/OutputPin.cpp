#include "OutputPin.hpp"


OutputPin::OutputPin(uint8_t pinNumber) {
  _pinNumber = pinNumber;
  pinMode(pinNumber, OUTPUT);
}


void OutputPin::on() {
  digitalWrite(_pinNumber, HIGH);
}


void OutputPin::off() {
  digitalWrite(_pinNumber, LOW);
}


void OutputPin::set(bool isOn) {
  digitalWrite(_pinNumber, isOn ? HIGH : LOW);
}

bool OutputPin::get() {
  return digitalRead(_pinNumber) == HIGH;
}


void OutputPin::toggle() {
  digitalWrite(_pinNumber, !get());
}