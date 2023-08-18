#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <TaskManager.h>
#include "CANMCP.hpp"
#include "LED.hpp"


namespace timer {
  void task20Hz();
}

namespace indicator {
  LED canReceive(1);
  LED loRaSend(4);
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
}

namespace data {
  float orientation_x, orientation_y, orientation_z;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;

  float outsideTemperature;
  float internalTemperature;
  float coldTemperature;
  float altitude;
  float climbRate;
}


void setup() {
  Serial.begin(115200);

  // ① 34ch 922.6E6
  // ② 30ch 921.8E6
  LoRa.begin(922.6E6);
  LoRa.setSignalBandwidth(500E3);

  connection::can.begin();
  connection::can.sendEvent(CANMCP::Publisher::AIR_DATA_COMMUNICATION_MODULE, CANMCP::EventCode::SETUP);

  Tasks.add(timer::task20Hz)->startFps(20);
}


void loop() {
  Tasks.update();

  // CANの受信処理
  if (connection::can.available()) {
    switch (connection::can.getLatestLabel()) {
    case CANMCP::Label::ORIENTATION:
      connection::can.receiveVector(&data::orientation_x, &data::orientation_y, &data::orientation_z);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::LINEAR_ACCELERATION:
      connection::can.receiveVector(&data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::ALTITUDE:
      connection::can.receiveScalar(&data::altitude);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::OUTSIDE_TEMPERATURE:
      connection::can.receiveScalar(&data::outsideTemperature);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::INTERNAL_TEMPERATURE:
      connection::can.receiveScalar(&data::internalTemperature);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::THERMO_COUPLE_TEMPERATURE:
      connection::can.receiveScalar(&data::coldTemperature);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::CLIMB_RATE:
      connection::can.receiveScalar(&data::climbRate);
      indicator::canReceive.toggle();
      break;
    }
  }
}


/// @brief 20Hzで実行したい処理
void timer::task20Hz() {
  // エアデータをダウンリンクで送信する
  const auto& airDataPacket = MsgPacketizer::encode(static_cast<uint8_t>(connection::Index::AIR_DATA),
    millis(),
    data::altitude,
    data::climbRate,
    data::outsideTemperature,
    data::internalTemperature,
    data::coldTemperature,
    data::orientation_x,
    data::orientation_y,
    data::orientation_z,
    data::linear_acceleration_x,
    data::linear_acceleration_y,
    data::linear_acceleration_z
  );

  connection::sendDownlink(airDataPacket.data.data(), airDataPacket.data.size());
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