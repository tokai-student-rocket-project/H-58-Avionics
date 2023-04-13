#include <SPI.h>
#include <TaskManager.h>
#include "mcp2515_can.h"


mcp2515_can can(7);


void setup() {
  Serial.begin(115200);

  can.begin(CAN_1000KBPS, MCP_16MHz);

  Tasks.add(task100Hz)->startIntervalMsec(10);
}


void loop() {
  Tasks.update();
}


void task100Hz() {
  uint8_t stmp[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  can.sendMsgBuf(0x00, 0, 0, 8, stmp);
}