#include "CANSTM.hpp"


void CANSTM::begin() {
  ACAN_STM32_Settings settings(250000);
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  settings.mTransmitPriority = ACAN_STM32_Settings::BY_REQUEST_ORDER;
  can.begin(settings);
}


bool CANSTM::available() {
  bool isAvailable = can.available0();

  if (isAvailable) {
    CANMessage message;
    can.receive0(message);

    _latestLabel = message.id;
    memcpy(_latestData, message.data, 8);
  }

  return isAvailable;
}


CANSTM::Label CANSTM::getLatestMessageLabel() {
  return static_cast<CANSTM::Label>(_latestLabel);
}


void CANSTM::sendSystemStatus(uint8_t mode, bool camera, bool sn3) {
  CANMessage message;
  message.id = static_cast<uint32_t>(Label::SYSTEM_STATUS);
  message.len = 3;

  message.data[0] = mode;
  message.data[1] = camera;
  message.data[2] = sn3;

  can.tryToSendReturnStatus(message);
}


void CANSTM::sendEvent(Publisher publisher, EventCode eventCode, uint32_t timestamp) {
  CANMessage message;
  message.id = static_cast<uint32_t>(Label::EVENT);
  message.len = 6;

  message.data[0] = static_cast<uint32_t>(publisher);
  message.data[1] = static_cast<uint32_t>(eventCode);
  memcpy(message.data + 2, &timestamp, 4);

  can.tryToSendReturnStatus(message);
}


void CANSTM::sendScalar(Label label, float value) {
  CANMessage message;
  message.id = static_cast<uint32_t>(label);
  message.len = 4;

  memcpy(message.data, &value, 4);

  can.tryToSendReturnStatus(message);
}


void CANSTM::sendVector(Label label, Axis axis, float value) {
  CANMessage message;
  message.id = static_cast<uint32_t>(label);
  message.len = 5;

  message.data[0] = static_cast<uint8_t>(axis);
  memcpy(message.data + 1, &value, 4);

  can.tryToSendReturnStatus(message);
}


void CANSTM::sendVector3D(Label label, float xValue, float yValue, float zValue) {
  sendVector(label, Axis::X, xValue);
  sendVector(label, Axis::Y, yValue);
  sendVector(label, Axis::Z, zValue);
}


void CANSTM::receiveStatus(uint8_t* mode, bool* camera, bool* sn3) {
  *mode = _latestData[0];
  *camera = _latestData[1];
  *sn3 = _latestData[2];
}


void CANSTM::receiveScalar(float* value) {
  memcpy(value, _latestData, 4);
}


void CANSTM::receiveVector(float* xValue, float* yValue, float* zValue) {
  float value;
  memcpy(&value, _latestData + 1, 4);

  uint8_t axis = _latestData[0];
  switch (axis) {
  case static_cast<uint8_t>(Axis::X):
    *xValue = value;
    break;
  case static_cast<uint8_t>(Axis::Y):
    *yValue = value;
    break;
  case static_cast<uint8_t>(Axis::Z):
    *zValue = value;
    break;
  }
}


void CANSTM::receiveSetReferencePressureCommand(float* referencePressure) {
  memcpy(referencePressure, _latestData, 4);
}