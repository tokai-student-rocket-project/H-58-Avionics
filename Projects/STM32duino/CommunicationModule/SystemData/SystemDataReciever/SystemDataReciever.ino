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
      transmitter::packet["PacketInfo"]["Sender"] = "SystemDataCommunicationModule";
      transmitter::packet["PacketInfo"]["Type"] = "SystemData";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      // transmitter::packet["FlightMode"] = mode;
      // transmitter::packet["Camera"] = camera;
      // transmitter::packet["SN3"] = sn3;
      // transmitter::packet["SupplyVoltage"] = voltage_supply;
      // transmitter::packet["BatteryVoltage"] = voltage_battery;
      // transmitter::packet["PoolVoltage"] = voltage_pool;
      // transmitter::packet["Latitude"] = latitude;
      // transmitter::packet["Longitude"] = longitude;

      serializeJson(transmitter::packet, Serial);
      Serial.println();
      transmitter::packet.clear();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  );

  MsgPacketizer::subscribe(LoRa, 0x01,
    [](
      uint8_t publisher,
      uint8_t eventCode,
      uint32_t timestamp
      )
    {
      transmitter::packet["PacketInfo"]["Sender"] = "SystemDataCommunicationModule";
      transmitter::packet["PacketInfo"]["Type"] = "Event";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();

      // switch (publisher) {
      // case 0: transmitter::packet["Publisher"] = "SensingModule"; break;
      // case 1: transmitter::packet["Publisher"] = "FlightModule"; break;
      // case 2: transmitter::packet["Publisher"] = "MissionModule"; break;
      // case 3: transmitter::packet["Publisher"] = "AirDataCommunicationModule"; break;
      // case 4: transmitter::packet["Publisher"] = "SystemDataCommunicationModule"; break;
      // }

      // switch (eventCode) {
      // case 0: transmitter::packet["EventCode"] = "SETUP"; break;
      // case 1: transmitter::packet["EventCode"] = "RESET"; break;
      // case 2: transmitter::packet["EventCode"] = "FLIGHT_MODE_ON"; break;
      // case 3: transmitter::packet["EventCode"] = "IGNITION"; break;
      // case 4: transmitter::packet["EventCode"] = "BURNOUT"; break;
      // case 5: transmitter::packet["EventCode"] = "APOGEE"; break;
      // case 6: transmitter::packet["EventCode"] = "SEPARATE"; break;
      // case 7: transmitter::packet["EventCode"] = "LAND"; break;
      // case 8: transmitter::packet["EventCode"] = "FLIGHT_MODE_OFF"; break;
      // case 9: transmitter::packet["EventCode"] = "FORCE_SEPARATE"; break;
      // case 10: transmitter::packet["EventCode"] = "REFERENCE_PRESSURE_UPDATED"; break;
      // }

      // transmitter::packet["Timestamp"] = timestamp;

      serializeJson(transmitter::packet, Serial);
      Serial.println();
      transmitter::packet.clear();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    });

  MsgPacketizer::subscribe(LoRa, 0x02,
    [](
      uint8_t publisher,
      uint8_t errorCode,
      uint8_t errorReason,
      uint32_t timestamp
      )
    {
      transmitter::packet["PacketInfo"]["Sender"] = "SystemDataCommunicationModule";
      transmitter::packet["PacketInfo"]["Type"] = "Error";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();

      // switch (publisher) {
      // case 0: transmitter::packet["Publisher"] = "SensingModule"; break;
      // case 1: transmitter::packet["Publisher"] = "FlightModule"; break;
      // case 2: transmitter::packet["Publisher"] = "MissionModule"; break;
      // case 3: transmitter::packet["Publisher"] = "AirDataCommunicationModule"; break;
      // case 4: transmitter::packet["Publisher"] = "SystemDataCommunicationModule"; break;
      // }

      // switch (errorCode) {
      // case 0: transmitter::packet["ErrorCode"] = "COMMAND_RECEIVE_FAILED"; break;
      // }

      // switch (errorReason) {
      // case 0: transmitter::packet["ErrorReason"] = "INVALID_KEY"; break;
      // }

      // transmitter::packet["Timestamp"] = timestamp;

      serializeJson(transmitter::packet, Serial);
      Serial.println();
      transmitter::packet.clear();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    });

  // デバッグ用 開始から10秒後に実行
  // Tasks.add([&]() {
  //   const auto& packet = MsgPacketizer::encode(0xF0, (uint8_t)0, (float)900.0);
  //   LoRa.beginPacket();
  //   LoRa.write(packet.data.data(), packet.data.size());
  //   LoRa.endPacket();
  //   })->startFps(3);
}


void loop() {
  Tasks.update();

  if (LoRa.parsePacket()) {
    MsgPacketizer::parse();
  }
}