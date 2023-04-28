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
  LoRa.begin(921.8E6);

  MsgPacketizer::subscribe(LoRa, 0x00,
    [](
      uint8_t mode,
      uint8_t camera,
      uint8_t separatorDrogue,
      uint8_t separatorMain,
      float latitude,
      float longitude
      )
    {
      transmitter::packet["mod"] = mode;
      transmitter::packet["cam"] = camera;
      transmitter::packet["spd"] = separatorDrogue;
      transmitter::packet["spm"] = separatorMain;
      transmitter::packet["lat"] = latitude;
      transmitter::packet["lon"] = longitude;

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