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


void CANMCP::receiveStatus(uint8_t* mode, bool* camera, bool* sn3) {
  *mode = _latestData[0];
  *camera = _latestData[1];
  *sn3 = _latestData[2];
}


void CANMCP::receiveScalar(float* value) {
  converter.data[0] = _latestData[0];
  converter.data[1] = _latestData[1];
  converter.data[2] = _latestData[2];
  converter.data[3] = _latestData[3];
  *value = converter.value;
}


void CANMCP::receiveVector(float* xValue, float* yValue, float* zValue) {
  converter.data[0] = _latestData[1];
  converter.data[1] = _latestData[2];
  converter.data[2] = _latestData[3];
  converter.data[3] = _latestData[4];

  uint8_t axis = _latestData[0];
  switch (axis) {
  case static_cast<uint8_t>(Axis::X):
    *xValue = converter.value;
    break;
  case static_cast<uint8_t>(Axis::Y):
    *yValue = converter.value;
    break;
  case static_cast<uint8_t>(Axis::Z):
    *zValue = converter.value;
    break;
  }
}


void CANMCP::receiveEvent(Publisher* publisher, EventCode* eventCode, uint32_t* time, uint16_t* payload) {
  *publisher = static_cast<CANMCP::Publisher>(_latestData[0]);
  *eventCode = static_cast<CANMCP::EventCode>(_latestData[1]);

  converter32.data[0] = _latestData[2];
  converter32.data[1] = _latestData[3];
  converter32.data[2] = _latestData[4];
  converter32.data[3] = _latestData[5];
  *time = converter32.value;

  converter16.data[0] = _latestData[6];
  converter16.data[1] = _latestData[7];
  *payload = converter16.value;
}