#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <TaskManager.h>
#include "CANMCP.hpp"
#include "LED.hpp"
#include "GNSS.hpp"
#include "Var.hpp"
#include "Transmitter.hpp"


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
  Transmitter transmitter;
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

  connection::transmitter.begin(921.8E6, 500E3);

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
  connection::transmitter.sendValveStatus(
    data::currentPosition,
    data::currentDesiredPosition,
    data::currentVelocity,
    data::mcuTemperature,
    data::motorTemperature,
    data::current,
    data::inputVoltage
  );

  connection::transmitter.sendPowerData(
    data::voltage_supply,
    data::voltage_battery,
    data::voltage_pool
  );

  indicator::loRaSend.toggle();
}


void timer::task10Hz() {
  if (sensor::gnss.available()) {
    data::latitude = sensor::gnss.getLatitude();
    data::longitude = sensor::gnss.getLongitude();

    indicator::gpsStatus.toggle();
  }

  connection::transmitter.sendPositionData(
    data::latitude,
    data::longitude
  );

  indicator::loRaSend.toggle();
}


void command::executeSetReferencePressureCommand(uint8_t key, float referencePressure) {
  if (key != command::innerKey) {
    connection::transmitter.sendError(
      static_cast<uint8_t>(CANMCP::Publisher::SYSTEM_DATA_COMMUNICATION_MODULE),
      static_cast<uint8_t>(CANMCP::ErrorCode::COMMAND_RECEIVE_FAILED),
      static_cast<uint8_t>(CANMCP::ErrorReason::INVALID_KEY),
      millis()
    );

    indicator::loRaSend.toggle();

    return;
  }

  connection::can.sendSetReferencePressureCommand(referencePressure);
  indicator::canSend.toggle();
}


void command::executeFlightModeOnCommand(uint8_t key) {
  if (key != command::innerKey) {
    connection::transmitter.sendError(
      static_cast<uint8_t>(CANMCP::Publisher::SYSTEM_DATA_COMMUNICATION_MODULE),
      static_cast<uint8_t>(CANMCP::ErrorCode::COMMAND_RECEIVE_FAILED),
      static_cast<uint8_t>(CANMCP::ErrorReason::INVALID_KEY),
      millis()
    );

    indicator::loRaSend.toggle();

    return;
  }

  connection::can.sendFlightModeOnCommand();
  indicator::canSend.toggle();
}


void connection::handleSystemStatus() {
  Var::FlightMode flightMode;
  Var::State cameraState, sn3State;
  bool doLogging;

  connection::can.receiveSystemStatus(&flightMode, &cameraState, &sn3State, &doLogging);

  connection::transmitter.sendSystemStatus(
    static_cast<uint8_t>(flightMode),
    static_cast<uint8_t>(cameraState),
    static_cast<uint8_t>(sn3State),
    doLogging
  );

  indicator::loRaSend.toggle();
}


void connection::handleSensingStatus() {
  float referencePressure;
  bool isSystemCalibrated, isGyroscopeCalibrated, isAccelerometerCalibrated, isMagnetometerCalibrated;

  connection::can.receiveSensingStatus(&referencePressure, &isSystemCalibrated, &isGyroscopeCalibrated, &isAccelerometerCalibrated, &isMagnetometerCalibrated);

  connection::transmitter.sendSensingStatus(
    referencePressure,
    isSystemCalibrated,
    isGyroscopeCalibrated,
    isAccelerometerCalibrated,
    isMagnetometerCalibrated
  );

  indicator::loRaSend.toggle();
}


void connection::handleEvent() {
  CANMCP::Publisher publisher;
  CANMCP::EventCode eventCode;
  uint32_t timestamp;

  connection::can.receiveEvent(&publisher, &eventCode, &timestamp);

  connection::transmitter.sendEvent(
    static_cast<uint8_t>(publisher),
    static_cast<uint8_t>(eventCode),
    timestamp
  );

  indicator::loRaSend.toggle();
}


void connection::handleError() {
  CANMCP::Publisher publisher;
  CANMCP::ErrorCode errorCode;
  CANMCP::ErrorReason errorReason;
  uint32_t timestamp;

  connection::can.receiveError(&publisher, &errorCode, &errorReason, &timestamp);

  connection::transmitter.sendError(
    static_cast<uint8_t>(publisher),
    static_cast<uint8_t>(errorCode),
    static_cast<uint8_t>(errorReason),
    timestamp
  );

  indicator::loRaSend.toggle();
}