#include "CANMCP.hpp"


CANMCP::CANMCP(uint8_t cs) {
  _can = new mcp2515_can(cs);
}


void CANMCP::begin() {
  _can->begin(CAN_250KBPS, MCP_8MHz);
}


bool CANMCP::available() {
  bool isAvailable = _can->checkReceive() == CAN_MSGAVAIL;

  if (isAvailable) {
    uint8_t len = 0;
    _can->readMsgBuf(&len, _latestData);
    _latestLabel = _can->getCanId();
  }

  return isAvailable;
}


CANMCP::Label CANMCP::getLatestLabel() {
  return static_cast<CANMCP::Label>(_latestLabel);
}


void CANMCP::sendEvent(Publisher publisher, EventCode eventCode, uint32_t timestamp) {
  uint8_t data[6];
  data[0] = static_cast<uint8_t>(publisher);
  data[1] = static_cast<uint8_t>(eventCode);
  memcpy(data + 2, &timestamp, 4);

  _can->sendMsgBuf(static_cast<uint32_t>(Label::EVENT), 0, 6, data);
}


void CANMCP::sendError(Publisher publisher, ErrorCode errorCode, ErrorReason errorReason, uint32_t timestamp) {
  uint8_t data[7];
  data[0] = static_cast<uint8_t>(publisher);
  data[1] = static_cast<uint8_t>(errorCode);
  data[2] = static_cast<uint8_t>(errorReason);
  memcpy(data + 3, &timestamp, 4);

  _can->sendMsgBuf(static_cast<uint32_t>(Label::ERROR), 0, 7, data);
}


void CANMCP::sendSetReferencePressureCommand(float payload) {
  uint8_t data[4];
  memcpy(data, &payload, 4);

  _can->sendMsgBuf(static_cast<uint32_t>(Label::SET_REFERENCE_PRESSURE_COMMAND), 0, 4, data);
}


void CANMCP::receiveSystemStatus(uint8_t* flightMode, bool* cameraState, bool* sn3State, bool* doLogging) {
  *flightMode = _latestData[0];
  *cameraState = _latestData[1];
  *sn3State = _latestData[2];
  *doLogging = _latestData[3];
}


void CANMCP::receiveSensingStatus(float* referencePressure, bool* isSystemCalibrated, bool* isGyroscopeCalibrated, bool* isAccelerometerCalibrated, bool* isMagnetometerCalibrated) {
  memcpy(referencePressure, _latestData, 4);
  *isSystemCalibrated = _latestData[4];
  *isGyroscopeCalibrated = _latestData[5];
  *isAccelerometerCalibrated = _latestData[6];
  *isMagnetometerCalibrated = _latestData[7];
}


void CANMCP::receiveScalar(float* value) {
  memcpy(value, _latestData, 4);
}


void CANMCP::receiveVector(float* xValue, float* yValue, float* zValue) {
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


void CANMCP::receiveEvent(Publisher* publisher, EventCode* eventCode, uint32_t* timestamp) {
  *publisher = static_cast<CANMCP::Publisher>(_latestData[0]);
  *eventCode = static_cast<CANMCP::EventCode>(_latestData[1]);
  memcpy(timestamp, _latestData + 2, 4);
}


void CANMCP::receiveError(Publisher* publisher, ErrorCode* errorCode, ErrorReason* errorReason, uint32_t* timestamp) {
  *publisher = static_cast<CANMCP::Publisher>(_latestData[0]);
  *errorCode = static_cast<CANMCP::ErrorCode>(_latestData[1]);
  *errorReason = static_cast<CANMCP::ErrorReason>(_latestData[2]);
  memcpy(timestamp, _latestData + 3, 4);
}