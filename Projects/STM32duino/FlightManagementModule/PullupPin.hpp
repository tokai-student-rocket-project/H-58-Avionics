#pragma once


#include <Arduino.h>


class PullupPin {
private:
  const uint16_t MINIMUM_COUNT = 10;

  uint8_t _pinNumber;
  uint16_t _openCount;
  uint16_t _closeCount;
  bool _isOpen;

public:
  PullupPin(uint8_t pinNumber);

  void update();
  bool isOpen();
};