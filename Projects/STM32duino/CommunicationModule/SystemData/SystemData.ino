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

namespace sensor {
  GNSS gnss;
}

namespace indicator {
  OutputPin canReceive(1);

  OutputPin loRaSend(4);

  OutputPin gpsStatus(5);
}

namespace interface {
  CANMCP can(7);
}

namespace data {
  uint8_t mode;
  bool camera, sn3, sn4;

  float voltage_supply, voltage_battery, voltage_pool;

  float latitude, longitude;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(500E3);

  sensor::gnss.begin();

  interface::can.begin();

  Tasks.add(timer::task10Hz)->startFps(10);
}


void loop() {
  Tasks.update();

  if (interface::can.available()) {
    switch (interface::can.getLatestLabel()) {
    case CANMCP::Label::STATUS:
      interface::can.receiveStatus(&data::mode, &data::camera, &data::sn3, &data::sn4);
      break;
    case CANMCP::Label::VOLTAGE_SUPPLY:
      interface::can.receiveScalar(&data::voltage_supply);
      break;
    case CANMCP::Label::VOLTAGE_BATTERY:
      interface::can.receiveScalar(&data::voltage_battery);
      break;
    case CANMCP::Label::VOLTAGE_POOL:
      interface::can.receiveScalar(&data::voltage_pool);
      break;
    }

    indicator::canReceive.toggle();
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
    data::mode,
    data::camera,
    data::sn3,
    data::sn4,
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