#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>


namespace transmitter {
  StaticJsonDocument<1024> packet;

  void receiveState(String label);
}


void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.begin(921.8E6);
}


void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    if (LoRa.available()) {
      transmitter::receiveState("mod");
      transmitter::receiveState("cam");
      transmitter::receiveState("spd");
      transmitter::receiveState("spm");
    }

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    serializeJson(transmitter::packet, Serial);
    Serial.println();
  }
}


void transmitter::receiveState(String label) {
  transmitter::packet[label] = LoRa.read();
}