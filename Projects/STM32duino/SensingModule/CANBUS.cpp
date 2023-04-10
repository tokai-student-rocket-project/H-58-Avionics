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


void CANBUS::sendVector(uint32_t id,
  uint8_t xLSB, uint8_t xMSB,
  uint8_t yLSB, uint8_t yMSB,
  uint8_t zLSB, uint8_t zMSB) {
  CANMessage message;
  message.id = id;
  message.len = 6;
  message.data[0] = xLSB;
  message.data[1] = xMSB;
  message.data[2] = yLSB;
  message.data[3] = yMSB;
  message.data[4] = zLSB;
  message.data[5] = zMSB;

  can.tryToSendReturnStatus(message);
}