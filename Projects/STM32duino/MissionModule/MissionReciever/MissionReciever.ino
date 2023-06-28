#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <ArduinoJson.h>


namespace transmitter {
  StaticJsonDocument<1024> packet;
}


void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.begin(925.8E6);
  LoRa.setSignalBandwidth(500E3);

  MsgPacketizer::subscribe(LoRa, 0x00,
    [](
      float acceleration_x,
      float acceleration_y,
      float acceleration_z
      )
    {
      transmitter::packet["tlm"]["rssi"] = LoRa.packetRssi();
      transmitter::packet["tlm"]["snr"] = LoRa.packetSnr();
      transmitter::packet["acc"]["x"] = acceleration_x;
      transmitter::packet["acc"]["y"] = acceleration_y;
      transmitter::packet["acc"]["z"] = acceleration_z;

      serializeJson(transmitter::packet, Serial);
      Serial.println();
      transmitter::packet.clear();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  );
}


void loop() {
  if (LoRa.parsePacket()) {
    MsgPacketizer::parse();
  }
}