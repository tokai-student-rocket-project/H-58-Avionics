#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <TaskManager.h>
#include "CANMCP.hpp"
#include "LED.hpp"
#include "GNSS.hpp"
#include "Var.hpp"


namespace timer {
  void task5Hz();
  void task10Hz();
}

namespace command {
  uint8_t innerKey = 0;

  void executeSetReferencePressureCommand(uint8_t key, float referencePressure);
  void executeFlightModeOnCommand(uint8_t key);
}

namespace sensor {
  GNSS gnss;
}

namespace indicator {
  LED canSend(2);
  LED canReceive(1);

  LED loRaSend(4);
  LED loRaReceive(3);

  LED gpsStatus(5);
}

namespace connection {
  enum class Index : uint8_t {
    AIR_DATA,
    POWER_DATA,
    GNSS_DATA,
    SYSTEM_STATUS,
    SENSING_STATUS,
    EVENT,
    ERROR,
    VALVE_STATUS,
    SET_REFERENCE_PRESSURE_COMMAND = 0xF0,
    SET_FLIGHT_MODE_ON
  };

  void sendDownlink(const uint8_t* data, uint32_t size);

  CANMCP can(7);


  void handleSystemStatus();
  void handleSensingStatus();
  void handleEvent();
  void handleError();
}

namespace data {
  float voltage_supply, voltage_battery, voltage_pool;
  float latitude, longitude;

  float currentPosition;
  float currentDesiredPosition;
  float currentVelocity;
  float mcuTemperature;
  float motorTemperature;
  float current;
  float inputVoltage;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(500E3);

  sensor::gnss.begin();

  connection::can.begin();
  connection::can.sendEvent(CANMCP::Publisher::SYSTEM_DATA_COMMUNICATION_MODULE, CANMCP::EventCode::SETUP);

  Tasks.add(timer::task5Hz)->startFps(5);
  Tasks.add(timer::task10Hz)->startFps(10);


  // 参照気圧設定コマンド
  MsgPacketizer::subscribe(LoRa, 0xF0, [](uint8_t key, float referencePressure) {
    indicator::loRaReceive.toggle();
    command::executeSetReferencePressureCommand(key, referencePressure);
    });

  // フライトモードオンコマンド
  MsgPacketizer::subscribe(LoRa, 0xF1, [](uint8_t key) {
    indicator::loRaReceive.toggle();
    command::executeFlightModeOnCommand(key);
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
    case CANMCP::Label::CURRENT_POSITION:
      connection::can.receiveServo(&data::currentPosition);
      Serial.println(data::currentPosition);
      break;
    case CANMCP::Label::CURRENT_DESIRED_POSITION:
      connection::can.receiveServo(&data::currentDesiredPosition);
      break;
    case CANMCP::Label::CURRENT_VELOCITY:
      connection::can.receiveServo(&data::currentVelocity);
      break;
    case CANMCP::Label::MCU_TEMPERATURE:
      connection::can.receiveServo(&data::mcuTemperature);
      break;
    case CANMCP::Label::MOTOR_TEMPERATURE:
      connection::can.receiveServo(&data::motorTemperature);
      break;
    case CANMCP::Label::CURRENT:
      connection::can.receiveServo(&data::current);
      break;
    case CANMCP::Label::INPUT_VOLTAGE:
      connection::can.receiveServo(&data::inputVoltage);
      break;
    }
  }
}


void timer::task5Hz() {
  const auto& valveStatusPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::VALVE_STATUS),
    data::currentPosition,
    data::currentDesiredPosition,
    data::currentVelocity,
    data::mcuTemperature,
    data::motorTemperature,
    data::current,
    data::inputVoltage
  );

  connection::sendDownlink(valveStatusPacket.data.data(), valveStatusPacket.data.size());

  const auto& powerDataPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::POWER_DATA),
    data::voltage_supply,
    data::voltage_battery,
    data::voltage_pool
  );

  connection::sendDownlink(powerDataPacket.data.data(), powerDataPacket.data.size());
}


void timer::task10Hz() {
  if (sensor::gnss.available()) {
    data::latitude = sensor::gnss.getLatitude();
    data::longitude = sensor::gnss.getLongitude();

    indicator::gpsStatus.toggle();
  }

  const auto& gnssDataPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::GNSS_DATA),
    data::latitude,
    data::longitude
  );

  connection::sendDownlink(gnssDataPacket.data.data(), gnssDataPacket.data.size());
}


void command::executeSetReferencePressureCommand(uint8_t key, float referencePressure) {
  if (key != command::innerKey) {
    const auto& errorPacket = MsgPacketizer::encode(
      static_cast<uint8_t>(connection::Index::ERROR),
      static_cast<uint8_t>(CANMCP::Publisher::SYSTEM_DATA_COMMUNICATION_MODULE),
      static_cast<uint8_t>(CANMCP::ErrorCode::COMMAND_RECEIVE_FAILED),
      static_cast<uint8_t>(CANMCP::ErrorReason::INVALID_KEY),
      millis()
    );

    connection::sendDownlink(errorPacket.data.data(), errorPacket.data.size());

    return;
  }

  connection::can.sendSetReferencePressureCommand(referencePressure);
  indicator::canSend.toggle();
}


void command::executeFlightModeOnCommand(uint8_t key) {
  if (key != command::innerKey) {
    const auto& errorPacket = MsgPacketizer::encode(
      static_cast<uint8_t>(connection::Index::ERROR),
      static_cast<uint8_t>(CANMCP::Publisher::SYSTEM_DATA_COMMUNICATION_MODULE),
      static_cast<uint8_t>(CANMCP::ErrorCode::COMMAND_RECEIVE_FAILED),
      static_cast<uint8_t>(CANMCP::ErrorReason::INVALID_KEY),
      millis()
    );

    connection::sendDownlink(errorPacket.data.data(), errorPacket.data.size());

    return;
  }

  connection::can.sendFlightModeOnCommand();
  indicator::canSend.toggle();
}


void connection::sendDownlink(const uint8_t* data, uint32_t size) {
  LoRa.beginPacket();
  LoRa.write(data, size);
  LoRa.endPacket();
  indicator::loRaSend.toggle();
}


void connection::handleSystemStatus() {
  Var::FlightMode flightMode;
  Var::State cameraState, sn3State;
  bool doLogging;

  connection::can.receiveSystemStatus(&flightMode, &cameraState, &sn3State, &doLogging);

  const auto& systemStatusPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::SYSTEM_STATUS),
    static_cast<uint8_t>(flightMode),
    static_cast<uint8_t>(cameraState),
    static_cast<uint8_t>(sn3State),
    doLogging
  );

  connection::sendDownlink(systemStatusPacket.data.data(), systemStatusPacket.data.size());
}


void connection::handleSensingStatus() {
  float referencePressure;
  bool isSystemCalibrated, isGyroscopeCalibrated, isAccelerometerCalibrated, isMagnetometerCalibrated;

  connection::can.receiveSensingStatus(&referencePressure, &isSystemCalibrated, &isGyroscopeCalibrated, &isAccelerometerCalibrated, &isMagnetometerCalibrated);

  const auto& sensingStatusPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::SENSING_STATUS),
    referencePressure,
    isSystemCalibrated,
    isGyroscopeCalibrated,
    isAccelerometerCalibrated,
    isMagnetometerCalibrated
  );

  connection::sendDownlink(sensingStatusPacket.data.data(), sensingStatusPacket.data.size());
}


void connection::handleEvent() {
  CANMCP::Publisher publisher;
  CANMCP::EventCode eventCode;
  uint32_t timestamp;

  connection::can.receiveEvent(&publisher, &eventCode, &timestamp);

  const auto& eventPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::EVENT),
    static_cast<uint8_t>(publisher),
    static_cast<uint8_t>(eventCode),
    timestamp
  );

  connection::sendDownlink(eventPacket.data.data(), eventPacket.data.size());
}


void connection::handleError() {
  CANMCP::Publisher publisher;
  CANMCP::ErrorCode errorCode;
  CANMCP::ErrorReason errorReason;
  uint32_t timestamp;

  connection::can.receiveError(&publisher, &errorCode, &errorReason, &timestamp);

  const auto& errorPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::ERROR),
    static_cast<uint8_t>(publisher),
    static_cast<uint8_t>(errorCode),
    static_cast<uint8_t>(errorReason),
    timestamp
  );

  connection::sendDownlink(errorPacket.data.data(), errorPacket.data.size());
}