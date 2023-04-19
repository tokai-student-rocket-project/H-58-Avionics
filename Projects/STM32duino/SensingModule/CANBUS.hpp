#pragma once

#include <ACAN_STM32.h>


class CANBUS {
public:
  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  void initialize();

  void send(uint32_t id, float value);
  void sendVector(uint32_t id, uint8_t axis, float value);
private:
};