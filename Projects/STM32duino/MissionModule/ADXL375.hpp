#pragma once


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL375.h>


class ADXL375 {
public:
  void begin();

  float get();

private:
  Adafruit_ADXL375 accel = Adafruit_ADXL375(-1, &Wire);
};