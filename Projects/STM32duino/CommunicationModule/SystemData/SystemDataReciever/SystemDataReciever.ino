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

  MsgPacketizer::subscribe(LoRa, 0x01,
    [](
      float voltage_supply,
      float voltage_battery,
      float voltage_pool
      )
    {
      transmitter::packet.clear();
      transmitter::packet["PacketInfo"]["Sender"] = "SystemDataCommunicationModule";
      transmitter::packet["PacketInfo"]["Type"] = "PowerData";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["SupplyVoltage"] = voltage_supply;
      transmitter::packet["BatteryVoltage"] = voltage_battery;
      transmitter::packet["PoolVoltage"] = voltage_pool;

      serializeJson(transmitter::packet, Serial);
      Serial.println();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  );

  MsgPacketizer::subscribe(LoRa, 0x02,
    [](
      float latitude,
      float longitude
      )
    {
      transmitter::packet.clear();
      transmitter::packet["PacketInfo"]["Sender"] = "SystemDataCommunicationModule";
      transmitter::packet["PacketInfo"]["Type"] = "PositionData";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["Latitude"] = latitude;
      transmitter::packet["Longitude"] = longitude;

      serializeJson(transmitter::packet, Serial);
      Serial.println();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  );

  MsgPacketizer::subscribe(LoRa, 0x03,
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

  MsgPacketizer::subscribe(LoRa, 0x04,
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

  MsgPacketizer::subscribe(LoRa, 0x05,
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

  MsgPacketizer::subscribe(LoRa, 0x06,
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

      // serializeJson(transmitter::packet, Serial);
      // Serial.println();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    });

  MsgPacketizer::subscribe(LoRa, 0x07,
    [](
      float currentPosition,
      float currentDesiredPosition,
      float currentVelocity,
      float mcuTemperature,
      float motorTemperature,
      float current,
      float inputVoltage
      )
    {
      transmitter::packet.clear();
      transmitter::packet["PacketInfo"]["Sender"] = "SystemDataCommunicationModule";
      transmitter::packet["PacketInfo"]["Type"] = "ValveStatus";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["CurrentPosition"] = currentPosition;
      transmitter::packet["CurrentDesiredPosition"] = currentDesiredPosition;
      transmitter::packet["CurrentVelocity"] = currentVelocity;
      transmitter::packet["McuTemperature"] = mcuTemperature;
      transmitter::packet["MotorTemperature"] = motorTemperature;
      transmitter::packet["Current"] = current;
      transmitter::packet["InputVoltage"] = inputVoltage;

      // serializeJson(transmitter::packet, Serial);
      // Serial.println();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    });
}


void loop() {
  Tasks.update();

  if (Serial.available()) {
    Serial.read();

    const auto& packet = MsgPacketizer::encode(0xF1, (uint8_t)0);
    LoRa.beginPacket();
    LoRa.write(packet.data.data(), packet.data.size());
    LoRa.endPacket();
  }

  if (LoRa.parsePacket()) {
    MsgPacketizer::parse();
  }
}