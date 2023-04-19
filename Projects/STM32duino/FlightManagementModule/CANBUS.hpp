#pragma once

#include <ACAN_STM32.h>


class CANBUS {
public:
  void initialize();

  void send(uint32_t id, float value);
  void sendVector(uint32_t id, float x, float y, float z);

  bool isAvailable();
  void receive();
private:
};