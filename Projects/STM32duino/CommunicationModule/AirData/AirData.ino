#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <TaskManager.h>
#include "CANMCP.hpp"
#include "OutputPin.hpp"


namespace timer {
  void task20Hz();
}

namespace indicator {
  OutputPin canReceive(1);

  OutputPin loRaSend(4);
}

namespace connection {
  CANMCP can(7);
}

namespace data {
  float orientation_x, orientation_y, orientation_z;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;

  float temperature;
  float altitude;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(923.8E6);
  LoRa.setSignalBandwidth(500E3);

  connection::can.begin();

  // connection::can.sendEvent(CANMCP::Publisher::AIR_DATA_COMMUNICATION_MODULE, CANMCP::EventCode::SETUP);

  Tasks.add(timer::task20Hz)->startFps(20);
}


void loop() {
  Tasks.update();

  if (connection::can.available()) {
    switch (connection::can.getLatestLabel()) {
    case CANMCP::Label::ORIENTATION:
      connection::can.receiveVector(&data::orientation_x, &data::orientation_y, &data::orientation_z);
      break;
    case CANMCP::Label::LINEAR_ACCELERATION:
      connection::can.receiveVector(&data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
      break;
    case CANMCP::Label::ALTITUDE:
      connection::can.receiveScalar(&data::altitude);
      break;
    case CANMCP::Label::TEMPERATURE:
      connection::can.receiveScalar(&data::temperature);
      break;
    }

    indicator::canReceive.toggle();
  }
}


void timer::task20Hz() {
  const auto& packet = MsgPacketizer::encode(
    0x00,
    data::altitude,
    data::temperature,
    data::orientation_x,
    data::orientation_y,
    data::orientation_z,
    data::linear_acceleration_x,
    data::linear_acceleration_y,
    data::linear_acceleration_z
  );

  LoRa.beginPacket();
  LoRa.write(packet.data.data(), packet.data.size());
  LoRa.endPacket();
  indicator::loRaSend.toggle();
}