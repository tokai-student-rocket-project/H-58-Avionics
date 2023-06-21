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

namespace interface {
  CANMCP can(7);
}

namespace data {
  float altitude;
  float orientation_x, orientation_y, orientation_z;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(923.8E6);
  LoRa.setSignalBandwidth(500E3);

  interface::can.begin();

  Tasks.add(timer::task20Hz)->startFps(20);
}


void loop() {
  Tasks.update();

  if (interface::can.available()) {
    switch (interface::can.getLatestLabel()) {
    case CANMCP::Label::ALTITUDE:
      interface::can.receiveScalar(&data::altitude);
      break;
    case CANMCP::Label::ORIENTATION:
      interface::can.receiveVector(&data::orientation_x, &data::orientation_y, &data::orientation_z);
      break;
    case CANMCP::Label::LINEAR_ACCELERATION:
      interface::can.receiveVector(&data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
      break;
    }

    indicator::canReceive.toggle();
  }
}


void timer::task20Hz() {
  const auto& packet = MsgPacketizer::encode(
    0x00,
    data::altitude,
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