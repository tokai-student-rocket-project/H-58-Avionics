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

namespace connection {
  CANMCP can(7);
}

namespace data {
  float internalAmbientTemperature;
  float supplyVoltage, batteryVoltage, poolVoltage;

  float latitude, longitude;
}


void setup() {
  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(250E3);

  connection::can.begin();

  Tasks.add(timer::task10Hz)->startFps(10);
}


void loop() {
  Tasks.update();

  // CAN受信処理
  if (connection::can.available()) {
    switch (connection::can.getLatestLabel()) {
    case CANMCP::Label::TEMPERATURE:
      connection::can.receiveScalar(&data::internalAmbientTemperature);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::VOLTAGE_SUPPLY:
      connection::can.receiveScalar(&data::supplyVoltage);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::VOLTAGE_BATTERY:
      connection::can.receiveScalar(&data::batteryVoltage);
      indicator::canReceive.toggle();
      break;
    case CANMCP::Label::VOLTAGE_POOL:
      connection::can.receiveScalar(&data::poolVoltage);
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
    millis(),
    data::internalAmbientTemperature,
    data::supplyVoltage,
    data::batteryVoltage,
    data::poolVoltage
  );

  LoRa.beginPacket();
  LoRa.write(packet.data.data(), packet.data.size());
  LoRa.endPacket();
  indicator::loRaSend.toggle();
}
