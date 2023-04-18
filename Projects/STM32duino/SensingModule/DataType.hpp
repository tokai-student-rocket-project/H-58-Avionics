#pragma once


struct raw_t {
  uint8_t XL;
  uint8_t L;
  uint8_t H;

  uint32_t combine() {
    return ((uint32_t)XL) | (((uint32_t)L) << 8) | (((uint32_t)H) << 16);
  }

  float toFloat(float lsb) {
    return (float)combine() / lsb;
  }
};