#include <LoRa.h>


void setup() {
  Serial.begin(115200);

  LoRa.begin(923.8E6);
  LoRa.setSignalBandwidth(500E3);

  pinMode(LED_BUILTIN, OUTPUT);
}


void loop() {
  if (LoRa.parsePacket()) {
    while (LoRa.available()) {
      LoRa.read();
    }

    Serial.println(LoRa.packetRssi());

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}