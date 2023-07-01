#pragma once


#include <Arduino.h>
#include <TaskManager.h>
#include "OutputPin.hpp"


class Buzzer {
public:
  Buzzer(uint8_t pinNumber, String identify);

  void beepOnce();
  void beepTwice();

  void beepLongOnce();

  void electricalParade();

  void beep2();
  void beep4();
  void beep8();

  void sleep4();
  void sleep8();

private:
  OutputPin* _pin;
  String _identify;
};