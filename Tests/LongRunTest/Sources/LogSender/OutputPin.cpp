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


void OutputPin::toggle() {
  digitalWrite(_pinNumber, !digitalRead(_pinNumber));
}