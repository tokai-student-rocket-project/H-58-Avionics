#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <TaskManager.h>
#include <ArduinoJson.h>


namespace transmitter {
  StaticJsonDocument<4096> packet;
}


void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(500E3);

  MsgPacketizer::subscribe(LoRa, 0x00,
    [](
      float voltage_supply,
      float voltage_battery,
      float voltage_pool,
      float latitude,
      float longitude
      )
    {
      transmitter::packet.clear();
      transmitter::packet["PacketInfo"]["Sender"] = "SystemDataCommunicationModule";
      transmitter::packet["PacketInfo"]["Type"] = "ValueData";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["SupplyVoltage"] = voltage_supply;
      transmitter::packet["BatteryVoltage"] = voltage_battery;
      transmitter::packet["PoolVoltage"] = voltage_pool;
      transmitter::packet["Latitude"] = latitude;
      transmitter::packet["Longitude"] = longitude;

      serializeJson(transmitter::packet, Serial);
      Serial.println();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  );

  MsgPacketizer::subscribe(LoRa, 0x01,
    [](
      uint8_t flightMode,
      bool cameraStatus,
      bool sn3Status,
      bool doLogging
      )
    {
      transmitter::packet.clear();
      transmitter::packet["PacketInfo"]["Sender"] = "SystemDataCommunicationModule";
      transmitter::packet["PacketInfo"]["Type"] = "SystemData";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["FlightMode"] = flightMode;
      transmitter::packet["CameraStatus"] = cameraStatus;
      transmitter::packet["SN3Status"] = sn3Status;
      transmitter::packet["DoLogging"] = doLogging;

      serializeJson(transmitter::packet, Serial);
      Serial.println();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  );

  MsgPacketizer::subscribe(LoRa, 0x02,
    [](
      float referencePressure,
      bool isSystemCalibrated,
      bool isGyroscopeCalibrated,
      bool isAccelerometerCalibrated,
      bool isMagnetometerCalibrated
      )
    {
      transmitter::packet.clear();
      transmitter::packet["PacketInfo"]["Sender"] = "SystemDataCommunicationModule";
      transmitter::packet["PacketInfo"]["Type"] = "SensingData";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["ReferencePressure"] = referencePressure;
      transmitter::packet["IsSystemCalibrated"] = isSystemCalibrated;
      transmitter::packet["IsGyroscopeCalibrated"] = isGyroscopeCalibrated;
      transmitter::packet["IsAccelerometerCalibrated"] = isAccelerometerCalibrated;
      transmitter::packet["IsMagnetometerCalibrated"] = isMagnetometerCalibrated;

      serializeJson(transmitter::packet, Serial);
      Serial.println();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  );

  MsgPacketizer::subscribe(LoRa, 0x03,
    [](
      uint8_t publisher,
      uint8_t eventCode,
      uint32_t timestamp
      )
    {
      transmitter::packet.clear();
      transmitter::packet["PacketInfo"]["Sender"] = "SystemDataCommunicationModule";
      transmitter::packet["PacketInfo"]["Type"] = "Event";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["Publisher"] = publisher;
      transmitter::packet["EventCode"] = eventCode;
      transmitter::packet["Timestamp"] = timestamp;

      serializeJson(transmitter::packet, Serial);
      Serial.println();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    });

  MsgPacketizer::subscribe(LoRa, 0x04,
    [](
      uint8_t publisher,
      uint8_t errorCode,
      uint8_t errorReason,
      uint32_t timestamp
      )
    {
      transmitter::packet.clear();
      transmitter::packet["PacketInfo"]["Sender"] = "SystemDataCommunicationModule";
      transmitter::packet["PacketInfo"]["Type"] = "Error";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["Publisher"] = publisher;
      transmitter::packet["ErrorCode"] = errorCode;
      transmitter::packet["ErrorReason"] = errorReason;
      transmitter::packet["Timestamp"] = timestamp;

      serializeJson(transmitter::packet, Serial);
      Serial.println();

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