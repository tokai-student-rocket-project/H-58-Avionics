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
  void lowRateDownlinkTask();
  void highRateDownlinkTask();
}

namespace command {
  // セキュリティ用の認証キー
  uint8_t innerKey = 0;

  void executeSetReferencePressureCommand(uint8_t key, float referencePressure);
  void executeFlightModeOnCommand(uint8_t key);
  void executeResetCommand(uint8_t key);
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
    VALVE_DATA,
    SET_REFERENCE_PRESSURE_COMMAND = 0xF0,
    SET_FLIGHT_MODE_ON
  };

  void reserveDownlink(const uint8_t* data, uint32_t size);
  void sendReservedDownlink();
  void sendDownlink(const uint8_t* data, uint32_t size);
  uint8_t reservedData[4096];
  uint32_t reservedSize = 0;
  bool isListenMode = false;

  CANMCP can(7);

  void handleEvent();
  void handleError();
}

namespace data {
  float latitude, longitude;

  float motorTemperature, mcuTemperature, current, inputVoltage;
  bool isWaiting;
  float currentPosition, currentDesiredPosition, currentVelocity;

  Var::FlightMode flightMode;
  Var::State cameraState, sn3State;
  bool doLogging;
  uint32_t flightTime;
  float voltage_supply, voltage_battery, voltage_pool;
  float referencePressure;
  bool isSystemCalibrated, isGyroscopeCalibrated, isAccelerometerCalibrated, isMagnetometerCalibrated;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(500E3);

  sensor::gnss.begin();

  connection::can.begin();
  connection::can.sendEvent(CANMCP::Publisher::SYSTEM_DATA_COMMUNICATION_MODULE, CANMCP::EventCode::SETUP);

  Tasks.add("lowRateDownlinkTask", timer::lowRateDownlinkTask)->startFps(5);
  Tasks.add("highRateDownlinkTask", timer::highRateDownlinkTask)->startFps(10);


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

  // リセットコマンド
  MsgPacketizer::subscribe(LoRa, 0xF2, [](uint8_t key) {
    indicator::loRaReceive.toggle();
    // 長くなるので処理は関数にまとめている
    command::executeResetCommand(key);
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
    case CANMCP::Label::SYSTEM_STATUS: {
      connection::can.receiveSystemStatus(&data::flightMode, &data::cameraState, &data::sn3State, &data::doLogging, &data::flightTime);
      indicator::canReceive.toggle();

      // コマンドを受信しやすようにSLEEPモードの時はダウンリンクの頻度を落とす
      if (data::flightMode == Var::FlightMode::SLEEP && connection::isListenMode == false) {
        Tasks["lowRateDownlinkTask"]->stop();
        Tasks["lowRateDownlinkTask"]->startFps(0.5);
        Tasks["highRateDownlinkTask"]->stop();
        Tasks["highRateDownlinkTask"]->startFps(0.9);
        connection::isListenMode = true;
      }

      if (data::flightMode != Var::FlightMode::SLEEP && connection::isListenMode == true) {
        Tasks["lowRateDownlinkTask"]->stop();
        Tasks["lowRateDownlinkTask"]->startFps(5);
        Tasks["highRateDownlinkTask"]->stop();
        Tasks["highRateDownlinkTask"]->startFps(9);
        connection::isListenMode = false;
      }

      break;
    }
    case CANMCP::Label::SENSING_STATUS:
      connection::can.receiveSensingStatus(&data::referencePressure, &data::isSystemCalibrated, &data::isGyroscopeCalibrated, &data::isAccelerometerCalibrated, &data::isMagnetometerCalibrated);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::VOLTAGE:
      connection::can.receiveVoltage(&data::voltage_supply, &data::voltage_pool, &data::voltage_battery);
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
    case CANMCP::Label::VALVE_DATA_1:
      connection::can.receiveValveData1(&data::motorTemperature, &data::mcuTemperature, &data::current, &data::inputVoltage);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::VALVE_DATA_2:
      connection::can.receiveValveData2(&data::currentPosition, &data::currentDesiredPosition, &data::currentVelocity);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::VALVE_MODE:
      connection::can.receiveValveMode(&data::isWaiting);
      indicator::canReceive.toggle();
      break;
    }
  }
}


/// @brief 5Hzで実行したい処理
void timer::lowRateDownlinkTask() {
  // バルブ情報をダウンリンクで送信する
  const auto& valveDataPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::VALVE_DATA),
    millis(),
    data::isWaiting,
    data::currentPosition,
    data::currentDesiredPosition,
    data::currentVelocity,
    data::mcuTemperature,
    data::motorTemperature,
    data::current,
    data::inputVoltage
  );

  connection::reserveDownlink(valveDataPacket.data.data(), valveDataPacket.data.size());

  // 電源情報をダウンリンクで送信する
  const auto& powerDataPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::POWER_DATA),
    millis(),
    data::voltage_supply,
    data::voltage_battery,
    data::voltage_pool
  );

  connection::reserveDownlink(powerDataPacket.data.data(), powerDataPacket.data.size());
}


/// @brief 10Hzで実行したい処理
void timer::highRateDownlinkTask() {
  // GNSS情報を受信する
  if (sensor::gnss.available()) {
    data::latitude = sensor::gnss.getLatitude();
    data::longitude = sensor::gnss.getLongitude();

    indicator::gpsStatus.toggle();

    // GNSS情報をダウンリンクで送信する
    const auto& gnssDataPacket = MsgPacketizer::encode(
      static_cast<uint8_t>(connection::Index::GNSS_DATA),
      millis(),
      data::latitude,
      data::longitude
    );

    connection::reserveDownlink(gnssDataPacket.data.data(), gnssDataPacket.data.size());
  }

  // システムステータスをダウンリンクで送信する
  const auto& systemStatusPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::SYSTEM_STATUS),
    millis(),
    static_cast<uint8_t>(data::flightMode),
    static_cast<bool>(data::cameraState),
    static_cast<bool>(data::sn3State),
    data::doLogging,
    data::flightTime
  );

  connection::reserveDownlink(systemStatusPacket.data.data(), systemStatusPacket.data.size());

  // 計測ステータスをダウンリンクで送信する
  const auto& sensingStatusPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::SENSING_STATUS),
    millis(),
    data::referencePressure,
    data::isSystemCalibrated,
    data::isGyroscopeCalibrated,
    data::isAccelerometerCalibrated,
    data::isMagnetometerCalibrated
  );

  connection::reserveDownlink(sensingStatusPacket.data.data(), sensingStatusPacket.data.size());
  connection::sendReservedDownlink();
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
      millis(),
      static_cast<uint8_t>(CANMCP::Publisher::SYSTEM_DATA_COMMUNICATION_MODULE),
      static_cast<uint8_t>(CANMCP::ErrorCode::COMMAND_RECEIVE_FAILED),
      static_cast<uint8_t>(CANMCP::ErrorReason::INVALID_KEY),
      millis()
    );

    connection::reserveDownlink(errorPacket.data.data(), errorPacket.data.size());

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
      millis(),
      static_cast<uint8_t>(CANMCP::Publisher::SYSTEM_DATA_COMMUNICATION_MODULE),
      static_cast<uint8_t>(CANMCP::ErrorCode::COMMAND_RECEIVE_FAILED),
      static_cast<uint8_t>(CANMCP::ErrorReason::INVALID_KEY),
      millis()
    );

    connection::reserveDownlink(errorPacket.data.data(), errorPacket.data.size());

    return;
  }

  // CANにフライトモードオンを送信する
  connection::can.sendFlightModeOn();
  indicator::canSend.toggle();
}


/// @brief リセットコマンドを処理する
/// @param key 認証キー
void command::executeResetCommand(uint8_t key) {
  // 認証キーが不正のときはエラーを送信して終わり
  if (key != command::innerKey) {
    // エラーをダウンリンクで送信する
    const auto& errorPacket = MsgPacketizer::encode(
      static_cast<uint8_t>(connection::Index::ERROR),
      millis(),
      static_cast<uint8_t>(CANMCP::Publisher::SYSTEM_DATA_COMMUNICATION_MODULE),
      static_cast<uint8_t>(CANMCP::ErrorCode::COMMAND_RECEIVE_FAILED),
      static_cast<uint8_t>(CANMCP::ErrorReason::INVALID_KEY),
      millis()
    );

    connection::reserveDownlink(errorPacket.data.data(), errorPacket.data.size());

    return;
  }

  // CANにリセットを送信する
  connection::can.sendReset();
  indicator::canSend.toggle();
}


void connection::reserveDownlink(const uint8_t* data, uint32_t size) {
  if (connection::reservedSize + size > 4096) {
    return;
  }

  memcpy(connection::reservedData + connection::reservedSize, data, size);
  connection::reservedSize += size;
}


void connection::sendReservedDownlink() {
  connection::sendDownlink(connection::reservedData, connection::reservedSize);
  connection::reservedSize = 0;
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


/// @brief CANのイベント受信処理をまとめた関数
void connection::handleEvent() {
  CANMCP::Publisher publisher;
  CANMCP::EventCode eventCode;
  uint32_t timestamp;

  connection::can.receiveEvent(&publisher, &eventCode, &timestamp);

  // イベントをそのままダウンリンクで送信
  const auto& eventPacket = MsgPacketizer::encode(
    static_cast<uint8_t>(connection::Index::EVENT),
    millis(),
    static_cast<uint8_t>(publisher),
    static_cast<uint8_t>(eventCode),
    timestamp
  );

  connection::reserveDownlink(eventPacket.data.data(), eventPacket.data.size());
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
    millis(),
    static_cast<uint8_t>(publisher),
    static_cast<uint8_t>(errorCode),
    static_cast<uint8_t>(errorReason),
    timestamp
  );

  connection::reserveDownlink(errorPacket.data.data(), errorPacket.data.size());
}