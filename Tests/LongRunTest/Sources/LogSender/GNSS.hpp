#pragma once


#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>


class GNSS {
public:
  void begin();

  bool available();

  float getLatitude();
  float getLongitude();

private:
  SFE_UBLOX_GNSS _gnss;
};