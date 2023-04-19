#include <CANBUS.hpp>


void CANBUS::initialize() {
  ACAN_STM32_Settings settings(1000000); // 1 Mbit/s
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);
}


void CANBUS::send(uint32_t id, float value) {
  converter.value = value;
  CANMessage message;
  message.id = id;
  message.len = 4;
  message.data[0] = converter.data[0];
  message.data[1] = converter.data[1];
  message.data[2] = converter.data[2];
  message.data[3] = converter.data[3];

  can.tryToSendReturnStatus(message);
}


void CANBUS::sendVector(uint32_t id, uint8_t axis, float value) {
  converter.value = value;
  CANMessage message;
  message.id = id;
  message.len = 5;
  message.data[0] = axis;
  message.data[1] = converter.data[0];
  message.data[2] = converter.data[1];
  message.data[3] = converter.data[2];
  message.data[4] = converter.data[3];

  can.tryToSendReturnStatus(message);
}