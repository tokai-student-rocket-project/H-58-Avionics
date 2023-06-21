#pragma once


#include <Arduino.h>
#include <TaskManager.h>
#include "OutputPin.hpp"


class Shiranui {
public:
  Shiranui(uint8_t pinNumber, String identify);

  void separate();

  bool get();

private:
  OutputPin* _pin;
  String _identify;
};