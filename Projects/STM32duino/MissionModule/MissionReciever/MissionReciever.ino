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

  MsgPacketizer::subscribe(LoRa, 0xAA,
    [](
      uint32_t millis,
      uint8_t flightMode,
      float x,
      float y,
      float z
      )
    {
      transmitter::packet["PacketInfo"]["Sender"] = "MissionModule";
      transmitter::packet["PacketInfo"]["Type"] = "MissionData";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["Millis"] = millis;
      transmitter::packet["FlightMode"] = flightMode;
      transmitter::packet["Acc"]["x"] = x;
      transmitter::packet["Acc"]["y"] = y;
      transmitter::packet["Acc"]["z"] = z;

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