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
      // transmitter::packet["PacketInfo"]["Sender"] = "AirDataCommunicationModule";
      // transmitter::packet["PacketInfo"]["Type"] = "AirData";
      // transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      // transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      // transmitter::packet["Alt"] = altitude;
      // transmitter::packet["Temp"] = temperature;
      // transmitter::packet["Ori"]["x"] = orientation_x;
      // transmitter::packet["Ori"]["y"] = orientation_y;
      // transmitter::packet["Ori"]["z"] = orientation_z;
      // transmitter::packet["Lia"]["x"] = linear_acceleration_x;
      // transmitter::packet["Lia"]["y"] = linear_acceleration_y;
      // transmitter::packet["Lia"]["z"] = linear_acceleration_z;

      // serializeJson(transmitter::packet, Serial);
      // Serial.println();
      // transmitter::packet.clear();

      Serial.print("AirDataCommunicationModule");
      Serial.print(",");
      Serial.print("AirData");
      Serial.print(",");
      Serial.print(LoRa.packetRssi());
      Serial.print(",");
      Serial.print(LoRa.packetSnr());
      Serial.print(",");
      Serial.print(millis());
      Serial.print(",");
      Serial.print(altitude);
      Serial.print(",");
      Serial.print(temperature);
      Serial.print(",");
      Serial.print(orientation_x);
      Serial.print(",");
      Serial.print(orientation_y);
      Serial.print(",");
      Serial.print(orientation_z);
      Serial.print(",");
      Serial.print(linear_acceleration_x);
      Serial.print(",");
      Serial.print(linear_acceleration_y);
      Serial.print(",");
      Serial.println(linear_acceleration_z);

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  );
}


void loop() {
  if (LoRa.parsePacket()) {
    MsgPacketizer::parse();
  }
}