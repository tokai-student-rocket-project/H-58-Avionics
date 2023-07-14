#include <TaskManager.h>
#include "Transmitter.hpp"
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
  Transmitter transmitter;
}

namespace data {
  float orientation_x, orientation_y, orientation_z;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;

  float outsideTemperature;
  float altitude;
}


void setup() {
  Serial.begin(115200);

  connection::transmitter.begin(923.8E6, 500E3);

  connection::can.begin();
  connection::can.sendEvent(CANMCP::Publisher::AIR_DATA_COMMUNICATION_MODULE, CANMCP::EventCode::SETUP);

  Tasks.add(timer::task20Hz)->startFps(20);
}


void loop() {
  Tasks.update();

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
    }
  }
}


void timer::task20Hz() {
  const auto& airDataPacket = MsgPacketizer::encode(static_cast<uint8_t>(connection::Index::AIR_DATA),
    data::altitude,
    data::outsideTemperature,
    data::orientation_x,
    data::orientation_y,
    data::orientation_z,
    data::linear_acceleration_x,
    data::linear_acceleration_y,
    data::linear_acceleration_z
  );

  connection::sendDownlink(airDataPacket.data.data(), airDataPacket.data.size());
}


void connection::sendDownlink(const uint8_t* data, uint32_t size) {
  LoRa.beginPacket();
  LoRa.write(data, size);
  LoRa.endPacket();
  indicator::loRaSend.toggle();
}