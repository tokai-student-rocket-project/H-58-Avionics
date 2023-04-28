#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>


namespace transmitter {
  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  StaticJsonDocument<1024> packet;

  void receiveState(String label);
  void receiveScalar(String label);
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

      transmitter::receiveScalar("lat");
      transmitter::receiveScalar("lon");
    }

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    serializeJson(transmitter::packet, Serial);
    Serial.println();
  }
}


void transmitter::receiveState(String label) {
  transmitter::packet[label] = LoRa.read();
}


void transmitter::receiveScalar(String label) {
  transmitter::converter.data[0] = LoRa.read();
  transmitter::converter.data[1] = LoRa.read();
  transmitter::converter.data[2] = LoRa.read();
  transmitter::converter.data[3] = LoRa.read();
  transmitter::packet[label] = transmitter::converter.value;
}