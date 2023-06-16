#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <mcp2515_can.h>
#include <TaskManager.h>
#include "OutputPin.hpp"


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
  void receiveScalar(uint8_t* data, float* value);
  void receiveVector(uint8_t* data, float* x, float* y, float* z);
}

namespace timer {
  void task10Hz();
}

namespace indicator {
  OutputPin canReceive(1);

  OutputPin loRaSend(4);
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

  canbus::initialize();

  Tasks.add(timer::task10Hz)->startFps(20);
}


void loop() {
  Tasks.update();

  if (CAN_MSGAVAIL == canbus::can.checkReceive()) {
    uint8_t len = 0;
    uint8_t data[8];

    canbus::can.readMsgBuf(&len, data);
    uint32_t id = canbus::can.getCanId();

    switch (id) {
    case static_cast<uint32_t>(canbus::Id::ALTITUDE):
      canbus::receiveScalar(data, &data::altitude);
      break;
    case static_cast<uint32_t>(canbus::Id::ORIENTATION):
      canbus::receiveVector(data, &data::orientation_x, &data::orientation_y, &data::orientation_z);
      break;
    case static_cast<uint32_t>(canbus::Id::LINEAR_ACCELERATION):
      canbus::receiveVector(data, &data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
      break;
    }
  }
}


void canbus::initialize() {
  canbus::can.begin(CAN_500KBPS, MCP_8MHz);
}


void canbus::receiveScalar(uint8_t* data, float* value) {
  canbus::converter.data[0] = data[0];
  canbus::converter.data[1] = data[1];
  canbus::converter.data[2] = data[2];
  canbus::converter.data[3] = data[3];
  *value = canbus::converter.value;

  indicator::canReceive.toggle();
}


void canbus::receiveVector(uint8_t* data, float* x, float* y, float* z) {
  canbus::converter.data[0] = data[1];
  canbus::converter.data[1] = data[2];
  canbus::converter.data[2] = data[3];
  canbus::converter.data[3] = data[4];

  switch (data[0]) {
  case static_cast<uint8_t>(canbus::Axis::X):
    *x = canbus::converter.value;
    break;
  case static_cast<uint8_t>(canbus::Axis::Y):
    *y = canbus::converter.value;
    break;
  case static_cast<uint8_t>(canbus::Axis::Z):
    *z = canbus::converter.value;
    break;
  }

  indicator::canReceive.toggle();
}


void timer::task10Hz() {
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