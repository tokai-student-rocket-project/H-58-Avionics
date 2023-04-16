#include <SPI.h>
#include <LoRa.h>


void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.begin(923.8E6);
}


void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.println(packetSize);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}