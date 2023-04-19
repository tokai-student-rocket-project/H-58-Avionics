#pragma once


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LPS2X.h>


class LPS33HW {
public:
  void begin();

  void getPressure(float* pressure);

private:
  Adafruit_LPS22 _lps;
};