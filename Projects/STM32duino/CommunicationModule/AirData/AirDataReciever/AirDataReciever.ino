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
  LoRa.begin(923.8E6);
  LoRa.setSignalBandwidth(500E3);

  MsgPacketizer::subscribe(LoRa, 0x00,
    [](
      float altitude,
      float temperature,
      float orientation_x,
      float orientation_y,
      float orientation_z,
      float linear_acceleration_x,
      float linear_acceleration_y,
      float linear_acceleration_z
      )
    {
      transmitter::packet["tlm"]["rssi"] = LoRa.packetRssi();
      transmitter::packet["tlm"]["snr"] = LoRa.packetSnr();
      transmitter::packet["alt"] = altitude;
      transmitter::packet["temp"] = temperature;
      transmitter::packet["ori"]["x"] = orientation_x;
      transmitter::packet["ori"]["y"] = orientation_y;
      transmitter::packet["ori"]["z"] = orientation_z;
      transmitter::packet["lia"]["x"] = linear_acceleration_x;
      transmitter::packet["lia"]["y"] = linear_acceleration_y;
      transmitter::packet["lia"]["z"] = linear_acceleration_z;

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