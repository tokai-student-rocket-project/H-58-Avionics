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

  // ① 38ch 923.4E6
  // ② 31ch 922.0E6
  LoRa.begin(923.4E6);
  LoRa.setSignalBandwidth(500E3);

  MsgPacketizer::subscribe(LoRa, 0xAA,
    [](
      uint32_t millis, uint8_t flightMode,
      uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1, uint8_t z0, uint8_t z1
      )
    {
      transmitter::packet["PacketInfo"]["Sender"] = "MM";
      transmitter::packet["PacketInfo"]["Type"] = "MissionData";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["Millis"] = millis;
      transmitter::packet["FlightMode"] = flightMode;
      transmitter::packet["Data"]["x0"] = x0;
      transmitter::packet["Data"]["x1"] = x1;
      transmitter::packet["Data"]["y0"] = y0;
      transmitter::packet["Data"]["y1"] = y1;
      transmitter::packet["Data"]["z0"] = z0;
      transmitter::packet["Data"]["z1"] = z1;

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