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
  void receiveScalar(uint8_t* data, float* value);
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

  float altitude;
  float temperature;
  float temperatureCold;

  float latitude;
  float longitude;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(500E3);

  sensor::gnss.begin();

  canbus::initialize();

  Tasks.add(timer::task10Hz)->startFps(10);
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
      Serial.println(data::mode);
      break;
    case 0x100:
      canbus::receiveScalar(data, &data::temperature);
      // Serial.print("temp: ");
      // Serial.println(data::temperature);
      break;
    case 0x101:
      canbus::receiveScalar(data, &data::temperatureCold);
      // Serial.print("cold: ");
      // Serial.println(data::temperatureCold);
      break;
    }
  }
}


void canbus::initialize() {
  canbus::can.begin(CAN_500KBPS, MCP_8MHz);
}


void canbus::receiveStatus(uint8_t* data, uint8_t* mode, uint8_t* camera, uint8_t* separatorDrogue, uint8_t* separatorMain) {
  *mode = data[0];
  *camera = data[1];
  *separatorDrogue = data[2];
  *separatorMain = data[3];

  indicator::canReceive.toggle();
}


void canbus::receiveScalar(uint8_t* data, float* value) {
  canbus::converter.data[0] = data[0];
  canbus::converter.data[1] = data[1];
  canbus::converter.data[2] = data[2];
  canbus::converter.data[3] = data[3];
  *value = canbus::converter.value;

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