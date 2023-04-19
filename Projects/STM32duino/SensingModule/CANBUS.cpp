#include <CANBUS.hpp>


void CANBUS::initialize() {
  ACAN_STM32_Settings settings(1000000); // 1 Mbit/s
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);
}


void CANBUS::send(uint32_t id, float value) {
  CANMessage message;
  message.id = id;
  message.len = 2;
  // message.data[0] = value.L;
  // message.data[1] = value.H;

  can.tryToSendReturnStatus(message);
}


void CANBUS::sendVector3(uint32_t id, float x, float y, float z) {
  CANMessage message;
  message.id = id;
  message.len = 6;
  // message.data[0] = x.L;
  // message.data[1] = x.H;
  // message.data[2] = y.L;
  // message.data[3] = y.H;
  // message.data[4] = z.L;
  // message.data[5] = z.H;

  can.tryToSendReturnStatus(message);
}


void CANBUS::sendVector4(uint32_t id, float w, float x, float y, float z) {
  CANMessage message;
  message.id = id;
  message.len = 8;
  // message.data[0] = w.L;
  // message.data[1] = w.H;
  // message.data[2] = x.L;
  // message.data[3] = x.H;
  // message.data[4] = y.L;
  // message.data[5] = y.H;
  // message.data[6] = z.L;
  // message.data[7] = z.H;

  can.tryToSendReturnStatus(message);
}