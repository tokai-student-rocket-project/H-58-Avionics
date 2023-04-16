#pragma once


struct raw_t {
  uint8_t LSB;
  uint8_t MSB;

  int16_t combine() {
    return ((int16_t)LSB) | (((int16_t)MSB) << 8);
  }

  float toFloat(float lsb) {
    return (float)combine() / lsb;
  }
};