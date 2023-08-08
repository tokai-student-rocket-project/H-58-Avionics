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
      transmitter::packet["PacketInfo"]["Sender"] = "MM";
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

  MsgPacketizer::subscribe(LoRa, 0xAB,
    [](
      uint32_t millis,
      uint8_t loggerUsage,
      float dataRate,
      uint32_t loggerOffset,
      uint32_t senderOffset,
      bool doLogging,
      bool doSending
      )
    {
      transmitter::packet.clear();
      transmitter::packet["PacketInfo"]["Sender"] = "MM";
      transmitter::packet["PacketInfo"]["Type"] = "MissionStatus";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["PacketInfo"]["Millis"] = millis;
      transmitter::packet["LoggerUsage"] = loggerUsage;
      transmitter::packet["DataRate"] = dataRate;
      transmitter::packet["LoggerOffset"] = loggerOffset;
      transmitter::packet["SenderOffset"] = senderOffset;
      transmitter::packet["DoLogging"] = doLogging;
      transmitter::packet["DoSending"] = doSending;

      serializeJson(transmitter::packet, Serial);
      Serial.println();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  );
}


void loop() {
  if (LoRa.parsePacket()) {
    MsgPacketizer::parse();
  }
}