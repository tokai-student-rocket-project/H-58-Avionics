#include <LoRa.h>
#include <TaskManager.h>


void setup() {
  Serial.begin(115200);

  LoRa.begin(923.8E6);
  LoRa.setSignalBandwidth(500E3);

  pinMode(LED_BUILTIN, OUTPUT);

  Tasks.add(sendTask)->startIntervalMsec(20);
}


void loop() {
  Tasks.update();
}


void sendTask() {
  uint8_t data[36];

  LoRa.beginPacket();
  LoRa.write(data, 36);
  LoRa.endPacket();

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}