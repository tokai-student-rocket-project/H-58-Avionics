#pragma once


#include <Arduino.h>


class Debugger {
public:
  void initialize();

  void printMessage(const String& message);
  void printScalar(const String& label, float value);
  void printVector(const String& label, float x, float y, float z);
};