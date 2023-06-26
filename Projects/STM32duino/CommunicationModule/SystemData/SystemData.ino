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
  uint8_t mode;
  bool camera, sn3;

  float voltage_supply, voltage_battery, voltage_pool;

  float latitude, longitude;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(500E3);

  sensor::gnss.begin();

  connection::can.begin();

  Tasks.add(timer::task10Hz)->startFps(10);
}


void loop() {
  Tasks.update();

  if (connection::can.available()) {
    switch (connection::can.getLatestLabel()) {
    case CANMCP::Label::SYSTEM_STATUS:
      connection::can.receiveStatus(&data::mode, &data::camera, &data::sn3);
      break;
    case CANMCP::Label::VOLTAGE_SUPPLY:
      connection::can.receiveScalar(&data::voltage_supply);
      break;
    case CANMCP::Label::VOLTAGE_BATTERY:
      connection::can.receiveScalar(&data::voltage_battery);
      break;
    case CANMCP::Label::VOLTAGE_POOL:
      connection::can.receiveScalar(&data::voltage_pool);
      break;
    case CANMCP::Label::EVENT:
      // DEBUG //////////////////////////////////////////////////////////////////////////////////
      CANMCP::Publisher publisher;
      CANMCP::EventCode eventCode;
      uint32_t time;
      uint16_t payload;
      connection::can.receiveEvent(&publisher, &eventCode, &time, &payload);

      switch (publisher) {
      case CANMCP::Publisher::SENSING_MODULE: Serial.print("SM "); break;
      case CANMCP::Publisher::FLIGHT_MODULE: Serial.print("FM "); break;
      case CANMCP::Publisher::MISSION_MODULE: Serial.print("MM "); break;
      case CANMCP::Publisher::AIR_DATA_COMMUNICATION_MODULE: Serial.print("ACM "); break;
      case CANMCP::Publisher::SYSTEM_DATA_COMMUNICATION_MODULE: Serial.print("SCM "); break;
      }

      switch (eventCode) {
      case CANMCP::EventCode::SETUP: Serial.print("SETUP "); break;
      case CANMCP::EventCode::RESET: Serial.print("RESET "); break;
      case CANMCP::EventCode::FLIGHT_MODE_ON: Serial.print("FLIGHT_MODE_ON "); break;
      case CANMCP::EventCode::IGNITION: Serial.print("IGNITION "); break;
      case CANMCP::EventCode::BURNOUT: Serial.print("BURNOUT "); break;
      case CANMCP::EventCode::APOGEE: Serial.print("APOGEE "); break;
      case CANMCP::EventCode::SEPARATE: Serial.print("SEPARATE "); break;
      case CANMCP::EventCode::LAND: Serial.print("LAND "); break;
      case CANMCP::EventCode::FLIGHT_MODE_OFF: Serial.print("FLIGHT_MODE_OFF "); break;
      }

      Serial.print((float)time / 1000.0, 2);
      Serial.print(" ");
      Serial.println(payload);
      // DEBUG //////////////////////////////////////////////////////////////////////////////////
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