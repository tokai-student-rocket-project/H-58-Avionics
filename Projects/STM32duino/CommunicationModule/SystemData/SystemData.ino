#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <mcp2515_can.h>
#include <TaskManager.h>
#include "OutputPin.hpp"
#include "GNSS.hpp"


namespace canbus {
  enum class Id : uint32_t {
    TEMPERATURE,
    PRESSURE,
    ALTITUDE,
    ACCELERATION,
    GYROSCOPE,
    MAGNETOMETER,
    ORIENTATION,
    LINEAR_ACCELERATION,
    GRAVITY,
    STATUS
  };

  enum class Axis : uint8_t {
    X,
    Y,
    Z
  };

  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  mcp2515_can can(7);

  void initialize();
  void receiveStatus(uint8_t* data, uint8_t* mode, uint8_t* camera, uint8_t* separatorDrogue, uint8_t* separatorMain);
}

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

namespace data {
  uint8_t mode;
  uint8_t camera;
  uint8_t separatorDrogue;
  uint8_t separatorMain;

  float latitude;
  float longitude;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(500E3);

  sensor::gnss.begin();

  canbus::initialize();

  Tasks.add(timer::task10Hz)->startIntervalMsec(100);
}


void loop() {
  Tasks.update();

  if (CAN_MSGAVAIL == canbus::can.checkReceive()) {
    uint8_t len = 0;
    uint8_t data[8];

    canbus::can.readMsgBuf(&len, data);
    uint32_t id = canbus::can.getCanId();

    switch (id) {
    case static_cast<uint32_t>(canbus::Id::STATUS):
      canbus::receiveStatus(data,
        &data::mode,
        &data::camera,
        &data::separatorDrogue,
        &data::separatorMain);
      break;
    }
  }
}


void canbus::initialize() {
  canbus::can.begin(CAN_1000KBPS, MCP_16MHz);
}


void canbus::receiveStatus(uint8_t* data, uint8_t* mode, uint8_t* camera, uint8_t* separatorDrogue, uint8_t* separatorMain) {
  *mode = data[0];
  *camera = data[1];
  *separatorDrogue = data[2];
  *separatorMain = data[3];

  indicator::canReceive.toggle();
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
    data::separatorDrogue,
    data::separatorMain,
    data::latitude,
    data::longitude
  );

  LoRa.beginPacket();
  LoRa.write(packet.data.data(), packet.data.size());
  LoRa.endPacket();
  indicator::loRaSend.toggle();
}