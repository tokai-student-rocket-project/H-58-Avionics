#pragma once


#include <Arduino.h>


class OutputPin {
private:
  uint8_t _pinNumber;

public:
  OutputPin(uint8_t pinNumber);

  void on();
  void off();
  void set(bool isOn);
  void toggle();
};