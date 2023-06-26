#include "CANMCP.hpp"


CANMCP::CANMCP(uint8_t cs) {
  _can = new mcp2515_can(cs);
}


void CANMCP::begin() {
  _can->begin(CAN_500KBPS, MCP_8MHz);
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


void CANMCP::sendEvent(Publisher publisher, EventCode eventCode, uint32_t time, uint16_t payload) {
  uint8_t data[8];
  data[0] = static_cast<uint32_t>(publisher);
  data[1] = static_cast<uint32_t>(eventCode);
  memcpy(data + 2, &time, 4);
  memcpy(data + 6, &payload, 2);

  _can->sendMsgBuf(static_cast<uint32_t>(Label::EVENT), 0, 8, data);
}


void CANMCP::receiveStatus(uint8_t* mode, bool* camera, bool* sn3) {
  *mode = _latestData[0];
  *camera = _latestData[1];
  *sn3 = _latestData[2];
}


void CANMCP::receiveScalar(float* value) {
  memcpy(&value, _latestData, 4);
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


void CANMCP::receiveEvent(Publisher* publisher, EventCode* eventCode, uint32_t* time, uint16_t* payload) {
  *publisher = static_cast<CANMCP::Publisher>(_latestData[0]);
  *eventCode = static_cast<CANMCP::EventCode>(_latestData[1]);
  memcpy(&time, _latestData + 2, 4);
  memcpy(&payload, _latestData + 6, 2);
}