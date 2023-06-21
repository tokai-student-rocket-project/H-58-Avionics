#include "CANSTM.hpp"


void CANSTM::begin(uint32_t bitrate) {
  ACAN_STM32_Settings settings(bitrate);
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);
}


bool CANSTM::available() {
  bool isAvailable = can.available0();

  if (isAvailable) {
    can.receive0(_latestMessage);
  }

  return isAvailable;
}


CANSTM::Label CANSTM::getLatestMessageLabel() {
  return (CANSTM::Label)_latestMessage.id;
}


void CANSTM::sendScalar(Label label, float value) {
  CANMessage message;
  message.id = static_cast<uint32_t>(label);
  message.len = 4;

  converter.value = value;
  message.data[0] = converter.data[0];
  message.data[1] = converter.data[1];
  message.data[2] = converter.data[2];
  message.data[3] = converter.data[3];

  can.tryToSendReturnStatus(message);
}


void CANSTM::sendVector(Label label, Axis axis, float value) {
  CANMessage message;
  message.id = static_cast<uint32_t>(label);
  message.len = 5;

  converter.value = value;
  message.data[0] = static_cast<uint8_t>(axis);
  message.data[1] = converter.data[0];
  message.data[2] = converter.data[1];
  message.data[3] = converter.data[2];
  message.data[4] = converter.data[3];

  can.tryToSendReturnStatus(message);
}


void CANSTM::sendVector3D(Label label, float xValue, float yValue, float zValue) {
  sendVector(label, Axis::X, xValue);
  sendVector(label, Axis::Y, yValue);
  sendVector(label, Axis::Z, zValue);
}


void CANSTM::receiveStatus(uint8_t* mode, bool* camera, bool* sn3, bool* sn4) {
  *mode = _latestMessage.data[0];
  *camera = _latestMessage.data[1];
  *sn3 = _latestMessage.data[2];
  *sn4 = _latestMessage.data[3];
}