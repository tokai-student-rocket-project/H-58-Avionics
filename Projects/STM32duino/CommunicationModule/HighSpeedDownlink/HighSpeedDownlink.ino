#include <SPI.h>
#include <LoRa.h>
#include <TaskManager.h>
#include "mcp2515_can.h"


mcp2515_can can(7);


void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.begin(923.8E6);

  can.begin(CAN_1000KBPS, MCP_16MHz);

  Tasks.add(task2Hz)->startIntervalMsec(500);
}


void loop() {
  Tasks.update();

  uint8_t len = 0;
  uint8_t data[6];

  if (CAN_MSGAVAIL == can.checkReceive()) {         // check if data coming
    can.readMsgBuf(&len, data);    // read data,  len: data length, buf: data buf

    int16_t xRaw = ((int16_t)data[0]) | (((int16_t)data[1]) << 8);
    int16_t yRaw = ((int16_t)data[2]) | (((int16_t)data[3]) << 8);
    int16_t zRaw = ((int16_t)data[4]) | (((int16_t)data[5]) << 8);

    Serial.print((float)xRaw / (float)16.0);
    Serial.print(",");
    Serial.print((float)yRaw / (float)16.0);
    Serial.print(",");
    Serial.println((float)zRaw / (float)16.0);
  }
}


void task2Hz() {
  uint8_t packet[240];

  LoRa.beginPacket();
  LoRa.write(packet, 240);
  LoRa.endPacket();

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  Serial.println("sended");
}