#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <TaskManager.h>
#include "CANMCP.hpp"
#include "OutputPin.hpp"
#include "GNSS.hpp"


namespace timer {
  void task10Hz();
}

namespace command {
  uint8_t innerKey = 0;

  void executeSetReferencePressureCommand(uint8_t key, float referencePressure);
}

namespace sensor {
  GNSS gnss;
}

namespace indicator {
  OutputPin canSend(2);
  OutputPin canReceive(1);

  OutputPin loRaSend(4);
  OutputPin loRaReceive(3);

  OutputPin gpsStatus(5);
}

namespace connection {
  CANMCP can(7);

  void handleSystemStatus();
  void handleSensingStatus();
  void handleEvent();
  void handleError();
}

namespace data {
  float voltage_supply, voltage_battery, voltage_pool;
  float latitude, longitude;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(500E3);

  sensor::gnss.begin();

  connection::can.begin();

  connection::can.sendEvent(CANMCP::Publisher::SYSTEM_DATA_COMMUNICATION_MODULE, CANMCP::EventCode::SETUP);

  Tasks.add(timer::task10Hz)->startFps(10);

  // 参照気圧設定コマンド
  MsgPacketizer::subscribe(LoRa, 0xF0, [](uint8_t key, float referencePressure) {
    indicator::loRaReceive.toggle();
    command::executeSetReferencePressureCommand(key, referencePressure);
    });
}


void loop() {
  Tasks.update();

  // LoRa受信処理
  if (LoRa.parsePacket()) {
    MsgPacketizer::parse();
  }

  // CAN受信処理
  if (connection::can.available()) {
    switch (connection::can.getLatestLabel()) {
    case CANMCP::Label::SYSTEM_STATUS:
      connection::handleSystemStatus();
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::SENSING_STATUS:
      connection::handleSensingStatus();
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::VOLTAGE_SUPPLY:
      connection::can.receiveScalar(&data::voltage_supply);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::VOLTAGE_BATTERY:
      connection::can.receiveScalar(&data::voltage_battery);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::VOLTAGE_POOL:
      connection::can.receiveScalar(&data::voltage_pool);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::EVENT:
      connection::handleEvent();
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::ERROR:
      connection::handleError();
      indicator::canReceive.toggle();
      break;
    }
  }
}


void timer::task10Hz() {
  if (sensor::gnss.available()) {
    data::latitude = sensor::gnss.getLatitude();
    data::longitude = sensor::gnss.getLongitude();

    indicator::gpsStatus.toggle();
  }

  const auto& packet = MsgPacketizer::encode(
    0x00,
    data::voltage_supply,
    data::voltage_battery,
    data::voltage_pool,
    data::latitude,
    data::longitude
  );

  LoRa.beginPacket();
  LoRa.write(packet.data.data(), packet.data.size());
  LoRa.endPacket();
  indicator::loRaSend.toggle();
}


void command::executeSetReferencePressureCommand(uint8_t key, float referencePressure) {
  if (key != command::innerKey) {
    const auto& packet = MsgPacketizer::encode(
      0x04,
      static_cast<uint8_t>(CANMCP::Publisher::SYSTEM_DATA_COMMUNICATION_MODULE),
      static_cast<uint8_t>(CANMCP::ErrorCode::COMMAND_RECEIVE_FAILED),
      static_cast<uint8_t>(CANMCP::ErrorReason::INVALID_KEY),
      millis()
    );

    LoRa.beginPacket();
    LoRa.write(packet.data.data(), packet.data.size());
    LoRa.endPacket();
    indicator::loRaSend.toggle();

    return;
  }

  connection::can.sendSetReferencePressureCommand(referencePressure);
  indicator::canSend.toggle();
}


void connection::handleSystemStatus() {
  uint8_t flightMode;
  bool cameraState, sn3State, doLogging;

  connection::can.receiveSystemStatus(&flightMode, &cameraState, &sn3State, &doLogging);

  const auto& packet = MsgPacketizer::encode(
    0x01,
    flightMode,
    cameraState,
    sn3State,
    doLogging
  );

  LoRa.beginPacket();
  LoRa.write(packet.data.data(), packet.data.size());
  LoRa.endPacket();
  indicator::loRaSend.toggle();
}


void connection::handleSensingStatus() {
  float referencePressure;
  bool isSystemCalibrated, isGyroscopeCalibrated, isAccelerometerCalibrated, isMagnetometerCalibrated;

  connection::can.receiveSensingStatus(&referencePressure, &isSystemCalibrated, &isGyroscopeCalibrated, &isAccelerometerCalibrated, &isMagnetometerCalibrated);

  const auto& packet = MsgPacketizer::encode(
    0x02,
    referencePressure,
    isSystemCalibrated,
    isGyroscopeCalibrated,
    isAccelerometerCalibrated,
    isMagnetometerCalibrated
  );

  LoRa.beginPacket();
  LoRa.write(packet.data.data(), packet.data.size());
  LoRa.endPacket();
  indicator::loRaSend.toggle();
}


void connection::handleEvent() {
  CANMCP::Publisher publisher;
  CANMCP::EventCode eventCode;
  uint32_t timestamp;

  connection::can.receiveEvent(&publisher, &eventCode, &timestamp);

  const auto& packet = MsgPacketizer::encode(
    0x03,
    static_cast<uint8_t>(publisher),
    static_cast<uint8_t>(eventCode),
    timestamp
  );

  LoRa.beginPacket();
  LoRa.write(packet.data.data(), packet.data.size());
  LoRa.endPacket();
  indicator::loRaSend.toggle();
}


void connection::handleError() {
  CANMCP::Publisher publisher;
  CANMCP::ErrorCode errorCode;
  CANMCP::ErrorReason errorReason;
  uint32_t timestamp;

  connection::can.receiveError(&publisher, &errorCode, &errorReason, &timestamp);

  const auto& packet = MsgPacketizer::encode(
    0x04,
    static_cast<uint8_t>(publisher),
    static_cast<uint8_t>(errorCode),
    static_cast<uint8_t>(errorReason),
    timestamp
  );

  LoRa.beginPacket();
  LoRa.write(packet.data.data(), packet.data.size());
  LoRa.endPacket();
  indicator::loRaSend.toggle();
}