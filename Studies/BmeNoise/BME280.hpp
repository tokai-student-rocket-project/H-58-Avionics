#pragma once


#include <Arduino.h>
#include <Wire.h>
#include <SparkFunBME280.h>


class BME {
public:
  void begin();

  void getPressure(float* pressure);

private:
  BME280 _bme280;
};