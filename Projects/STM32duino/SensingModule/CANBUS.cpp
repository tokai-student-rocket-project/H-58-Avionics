#include <CANBUS.hpp>


void CANBUS::initialize() {
  ACAN_STM32_Settings settings(1000000); // 1 Mbit/s
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);
}


void CANBUS::send(uint32_t id, double value) {
  uint32_t valueRaw = (uint32_t)(value * 4096.0);

  CANMessage message;
  message.id = id;
  message.len = 4;
  message.data[0] = (uint8_t)((valueRaw & 0xFF000000) >> 24);
  message.data[1] = (uint8_t)((valueRaw & 0x00FF0000) >> 16);
  message.data[2] = (uint8_t)((valueRaw & 0x0000FF00) >> 8);
  message.data[3] = (uint8_t)((valueRaw & 0x000000FF) >> 0);

  can.tryToSendReturnStatus(message);
}