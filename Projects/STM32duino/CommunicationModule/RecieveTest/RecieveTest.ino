#include <SPI.h>
#include <LoRa.h>


namespace transmitter {
  union Converter {
    float value;
    uint8_t data[4];
  }converter;
}


void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.begin(923.8E6);
}


void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      transmitter::converter.data[0] = LoRa.read();
      transmitter::converter.data[1] = LoRa.read();
      transmitter::converter.data[2] = LoRa.read();
      transmitter::converter.data[3] = LoRa.read();
      Serial.println(transmitter::converter.value);
    }

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}