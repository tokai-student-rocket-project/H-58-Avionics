#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>


namespace transmitter {
  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  StaticJsonDocument<1024> packet;

  void receiveScalar(String label);
  void receiveVector(String label, String axis);
}


void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.begin(923.8E6);
}


void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    if (LoRa.available()) {
      transmitter::receiveScalar("alt");

      transmitter::receiveVector("acc", "x");
      transmitter::receiveVector("acc", "y");
      transmitter::receiveVector("acc", "z");

      transmitter::receiveVector("mag", "x");
      transmitter::receiveVector("mag", "y");
      transmitter::receiveVector("mag", "z");

      transmitter::receiveVector("gyr", "x");
      transmitter::receiveVector("gyr", "y");
      transmitter::receiveVector("gyr", "z");

      transmitter::receiveVector("ori", "x");
      transmitter::receiveVector("ori", "y");
      transmitter::receiveVector("ori", "z");

      transmitter::receiveVector("lac", "x");
      transmitter::receiveVector("lac", "y");
      transmitter::receiveVector("lac", "z");
    }

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    serializeJson(transmitter::packet, Serial);
    Serial.println();
  }
}


void transmitter::receiveScalar(String label) {
  transmitter::converter.data[0] = LoRa.read();
  transmitter::converter.data[1] = LoRa.read();
  transmitter::converter.data[2] = LoRa.read();
  transmitter::converter.data[3] = LoRa.read();
  transmitter::packet[label] = transmitter::converter.value;
}


void transmitter::receiveVector(String label, String axis) {
  transmitter::converter.data[0] = LoRa.read();
  transmitter::converter.data[1] = LoRa.read();
  transmitter::converter.data[2] = LoRa.read();
  transmitter::converter.data[3] = LoRa.read();
  transmitter::packet[label][axis] = transmitter::converter.value;
}
