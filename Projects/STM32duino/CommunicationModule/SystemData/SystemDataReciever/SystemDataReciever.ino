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
  LoRa.setSignalBandwidth(500E3);

  MsgPacketizer::subscribe(LoRa, 0x00,
    [](
      uint8_t mode,
      bool camera,
      bool sn3,
      float voltage_supply,
      float voltage_battery,
      float voltage_pool,
      float latitude,
      float longitude
      )
    {
      transmitter::packet["tlm"]["rssi"] = LoRa.packetRssi();
      transmitter::packet["tlm"]["snr"] = LoRa.packetSnr();
      transmitter::packet["mod"] = mode;
      transmitter::packet["cam"] = camera;
      transmitter::packet["sn3"] = sn3;
      transmitter::packet["vsp"] = voltage_supply;
      transmitter::packet["vbt"] = voltage_battery;
      transmitter::packet["vpl"] = voltage_pool;
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