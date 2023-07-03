#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <TaskManager.h>
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

  // デバッグ用 開始から10秒後に実行
  Tasks.add([&]() {
    const auto& packet = MsgPacketizer::encode(0xF0, (uint8_t)0, (float)900.0);

    LoRa.beginPacket();
    LoRa.write(packet.data.data(), packet.data.size());
    LoRa.endPacket();
    })->startOnceAfterSec(10);

    MsgPacketizer::subscribe(LoRa, 0x01,
      [](
        uint8_t publisher,
        uint8_t eventCode,
        uint32_t timestamp
        )
      {
        transmitter::packet["tlm"]["rssi"] = LoRa.packetRssi();
        transmitter::packet["tlm"]["snr"] = LoRa.packetSnr();

        switch (publisher) {
        case 0: transmitter::packet["pub"] = "SM"; break;
        case 1: transmitter::packet["pub"] = "FM"; break;
        case 2: transmitter::packet["pub"] = "MM"; break;
        case 3: transmitter::packet["pub"] = "ACM"; break;
        case 4: transmitter::packet["pub"] = "SCM"; break;
        }

        switch (eventCode) {
        case 0: transmitter::packet["ecd"] = "SETUP"; break;
        case 1: transmitter::packet["ecd"] = "RESET"; break;
        case 2: transmitter::packet["ecd"] = "FLIGHT_MODE_ON"; break;
        case 3: transmitter::packet["ecd"] = "IGNITION"; break;
        case 4: transmitter::packet["ecd"] = "BURNOUT"; break;
        case 5: transmitter::packet["ecd"] = "APOGEE"; break;
        case 6: transmitter::packet["ecd"] = "SEPARATE"; break;
        case 7: transmitter::packet["ecd"] = "LAND"; break;
        case 8: transmitter::packet["ecd"] = "FLIGHT_MODE_OFF"; break;
        case 9: transmitter::packet["ecd"] = "FORCE_SEPARATE"; break;
        }

        transmitter::packet["tim"] = timestamp;

        serializeJson(transmitter::packet, Serial);
        Serial.println();
        transmitter::packet.clear();

        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      });
}


void loop() {
  Tasks.update();

  if (LoRa.parsePacket()) {
    MsgPacketizer::parse();
  }
}