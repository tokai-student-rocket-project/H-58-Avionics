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
  // セキュリティ用の認証キー
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


  void handleVoltage();
  void handleSystemStatus();
  void handleSensingStatus();
  void handleEvent();
  void handleError();
}

namespace data {
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
    // 長くなるので処理は関数にまとめている
    command::executeSetReferencePressureCommand(key, referencePressure);
    });

  // フライトモードオンコマンド
  MsgPacketizer::subscribe(LoRa, 0xF1, [](uint8_t key) {
    indicator::loRaReceive.toggle();
    // 長くなるので処理は関数にまとめている
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
    case CANMCP::Label::VOLTAGE:
      connection::handleVoltage();
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


/// @brief 5Hzで実行したい処理
void timer::task5Hz() {
  // バルブ情報をダウンリンクで送信する
  // const auto& valveStatusPacket = MsgPacketizer::encode(
  //   static_cast<uint8_t>(connection::Index::VALVE_STATUS),
  //   data::currentPosition,
  //   data::currentDesiredPosition,
  //   data::currentVelocity,
  //   data::mcuTemperature,
  //   data::motorTemperature,
  //   data::current,
  //   data::inputVoltage
  // );

  // connection::sendDownlink(valveStatusPacket.data.data(), valveStatusPacket.data.size());
}


/// @brief 10Hzで実行したい処理
void timer::task10Hz() {
  // GNSS情報を受信する
  if (sensor::gnss.available()) {
    data::latitude = sensor::gnss.getLatitude();
    data::longitude = sensor::gnss.getLongitude();

    indicator::gpsStatus.toggle();
  }

  // GNSS情報をダウンリンクで送信する
  const auto& gnssDataPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::GNSS_DATA),
    data::latitude,
    data::longitude
  );

  connection::sendDownlink(gnssDataPacket.data.data(), gnssDataPacket.data.size());
}


/// @brief 参照気圧設定コマンドを処理する
/// @param key 認証キー
/// @param referencePressure 設定する参照気圧 [hPa]
void command::executeSetReferencePressureCommand(uint8_t key, float referencePressure) {
  // 認証キーが不正のときはエラーを送信して終わり
  if (key != command::innerKey) {
    // エラーをダウンリンクで送信する
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

  // CANに参照気圧設定を送信する
  connection::can.sendSetReferencePressure(referencePressure);
  indicator::canSend.toggle();
}


/// @brief フライトモードオンコマンドを処理する
/// @param key 認証キー
void command::executeFlightModeOnCommand(uint8_t key) {
  // 認証キーが不正のときはエラーを送信して終わり
  if (key != command::innerKey) {
    // エラーをダウンリンクで送信する
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

  // CANにフライトモードオンを送信する
  connection::can.sendFlightModeOn();
  indicator::canSend.toggle();
}


/// @brief LoRaの送信処理をまとめた関数
/// @param data 送信するデータ配列 イミュータブル
/// @param size 送信するデータ長
void connection::sendDownlink(const uint8_t* data, uint32_t size) {
  LoRa.beginPacket();
  LoRa.write(data, size);
  LoRa.endPacket();
  indicator::loRaSend.toggle();
}


/// @brief CANの電圧受信処理をまとめた関数
void connection::handleVoltage() {
  float voltage_supply, voltage_battery, voltage_pool;

  connection::can.receiveVoltage(&voltage_supply, &voltage_pool, &voltage_battery);


  // 電源情報をそのままダウンリンクで送信
  const auto& powerDataPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::POWER_DATA),
    voltage_supply,
    voltage_battery,
    voltage_pool
  );

  connection::sendDownlink(powerDataPacket.data.data(), powerDataPacket.data.size());
}


/// @brief CANのシステムステータス受信処理をまとめた関数
void connection::handleSystemStatus() {
  Var::FlightMode flightMode;
  Var::State cameraState, sn3State;
  bool doLogging;

  connection::can.receiveSystemStatus(&flightMode, &cameraState, &sn3State, &doLogging);

  // システムステータスをそのままダウンリンクで送信
  const auto& systemStatusPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::SYSTEM_STATUS),
    static_cast<uint8_t>(flightMode),
    static_cast<uint8_t>(cameraState),
    static_cast<uint8_t>(sn3State),
    doLogging
  );

  connection::sendDownlink(systemStatusPacket.data.data(), systemStatusPacket.data.size());
}


/// @brief CANの計測ステータス受信処理をまとめた関数
void connection::handleSensingStatus() {
  float referencePressure;
  bool isSystemCalibrated, isGyroscopeCalibrated, isAccelerometerCalibrated, isMagnetometerCalibrated;

  connection::can.receiveSensingStatus(&referencePressure, &isSystemCalibrated, &isGyroscopeCalibrated, &isAccelerometerCalibrated, &isMagnetometerCalibrated);

  // 計測ステータスをそのままダウンリンクで送信
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


/// @brief CANのイベント受信処理をまとめた関数
void connection::handleEvent() {
  CANMCP::Publisher publisher;
  CANMCP::EventCode eventCode;
  uint32_t timestamp;

  connection::can.receiveEvent(&publisher, &eventCode, &timestamp);

  // イベントをそのままダウンリンクで送信
  const auto& eventPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::EVENT),
    static_cast<uint8_t>(publisher),
    static_cast<uint8_t>(eventCode),
    timestamp
  );

  connection::sendDownlink(eventPacket.data.data(), eventPacket.data.size());
}


/// @brief CANのエラー受信処理をまとめた関数
void connection::handleError() {
  CANMCP::Publisher publisher;
  CANMCP::ErrorCode errorCode;
  CANMCP::ErrorReason errorReason;
  uint32_t timestamp;

  connection::can.receiveError(&publisher, &errorCode, &errorReason, &timestamp);

  // エラーをそのままダウンリンクで送信
  const auto& errorPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::ERROR),
    static_cast<uint8_t>(publisher),
    static_cast<uint8_t>(errorCode),
    static_cast<uint8_t>(errorReason),
    timestamp
  );

  connection::sendDownlink(errorPacket.data.data(), errorPacket.data.size());
}