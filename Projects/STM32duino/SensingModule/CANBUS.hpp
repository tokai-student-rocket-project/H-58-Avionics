#pragma once

#include <ACAN_STM32.h>


class CANBUS {
public:
  void initialize();
  void send(uint32_t id, float value);
  void sendVector3(uint32_t id, float x, float y, float z);
  void sendVector4(uint32_t id, float w, float x, float y, float z);
private:
};