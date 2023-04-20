#pragma once


#include <Arduino.h>


class PullupPin {
public:
  PullupPin(uint8_t pinNumber);

  bool isOpen();

private:
  uint8_t _pinNumber;
};