#pragma once


#include <Arduino.h>
#include <TaskManager.h>
#include "OutputPin.hpp"


class Blinker {
public:
  Blinker(uint8_t pinNumber, String identify);

  void startBlink(float fps);
  void stopBlink();

  void on();
  void off();

private:
  OutputPin* _pin;
  String _identify;
};