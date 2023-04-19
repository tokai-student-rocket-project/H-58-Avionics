#include <CANBUS.hpp>


void CANBUS::initialize() {
  ACAN_STM32_Settings settings(1000000); // 1 Mbit/s
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);
}


void CANBUS::send(uint32_t id, float value) {
  int32_t rawValue = value * 10.0;

  CANMessage message;
  message.id = id;
  message.len = 2;
  message.data[0] = (uint8_t)(rawValue);
  message.data[1] = (uint8_t)(rawValue >> 8);

  can.tryToSendReturnStatus(message);
}


void CANBUS::sendVector(uint32_t id, float x, float y, float z) {
  int32_t xValue = x * 10.0;
  int32_t yValue = y * 10.0;
  int32_t zValue = z * 10.0;

  CANMessage message;
  message.id = id;
  message.len = 6;
  message.data[0] = (uint8_t)(xValue);
  message.data[1] = (uint8_t)(xValue >> 8);
  message.data[2] = (uint8_t)(yValue);
  message.data[3] = (uint8_t)(yValue >> 8);
  message.data[4] = (uint8_t)(zValue);
  message.data[5] = (uint8_t)(zValue >> 8);

  can.tryToSendReturnStatus(message);
}


bool CANBUS::isAvailable() {
  return can.available0();
}


void CANBUS::receive() {
  CANMessage message;
  can.receive0(message);

  if (message.id == 0x01) {
    int32_t rawValue = (int32_t)message.data[0] | ((int32_t)message.data[1]) << 8;
    float value = (float)rawValue / 10.0;
    Serial.println(value);
  }
}