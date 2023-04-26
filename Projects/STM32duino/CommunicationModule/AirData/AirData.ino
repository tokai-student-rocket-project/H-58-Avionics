#include <SPI.h>
#include <LoRa.h>
#include <mcp2515_can.h>
#include <TaskManager.h>
#include "OutputPin.hpp"


namespace canbus {
  enum class Id: uint32_t {
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

  enum class Axis: uint8_t {
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
  OutputPin ledCanReceive(LED_BUILTIN);
}

namespace transmitter {
  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  uint8_t downlinkData[256];
  uint8_t offset = 0;

  void reserve(float value);
  void send();
}

namespace data {
  float altitude;
  float acceleration_x, acceleration_y, acceleration_z;
  float magnetometer_x, magnetometer_y, magnetometer_z;
  float gyroscope_x, gyroscope_y, gyroscope_z;
  float orientation_x, orientation_y, orientation_z;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
}


void setup() {
  Serial.begin(115200);
  LoRa.begin(923.8E6);

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
    case static_cast<uint32_t>(canbus::Id::ALTITUDE):
      canbus::receiveScalar(data, &data::altitude);
      break;
    case static_cast<uint32_t>(canbus::Id::ACCELERATION):
      canbus::receiveVector(data, &data::acceleration_x, &data::acceleration_y, &data::acceleration_z);
      break;
    case static_cast<uint32_t>(canbus::Id::MAGNETOMETER):
      canbus::receiveVector(data, &data::magnetometer_x, &data::magnetometer_y, &data::magnetometer_z);
      break;
    case static_cast<uint32_t>(canbus::Id::GYROSCOPE):
      canbus::receiveVector(data, &data::gyroscope_x, &data::gyroscope_y, &data::gyroscope_z);
      break;
    case static_cast<uint32_t>(canbus::Id::ORIENTATION):
      canbus::receiveVector(data, &data::orientation_x, &data::orientation_y, &data::orientation_z);
      break;
    case static_cast<uint32_t>(canbus::Id::LINEAR_ACCELERATION):
      canbus::receiveVector(data, &data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
      break;
    }

    indicator::ledCanReceive.toggle();
  }
}


void canbus::initialize() {
  canbus::can.begin(CAN_1000KBPS, MCP_16MHz);
}


void canbus::receiveScalar(uint8_t* data, float* value) {
  canbus::converter.data[0] = data[1];
  canbus::converter.data[1] = data[2];
  canbus::converter.data[2] = data[3];
  canbus::converter.data[3] = data[4];
  *value = canbus::converter.value;
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
}


void timer::task10Hz() {
  transmitter::reserve(data::altitude);

  transmitter::reserve(data::acceleration_x);
  transmitter::reserve(data::acceleration_y);
  transmitter::reserve(data::acceleration_z);

  transmitter::reserve(data::magnetometer_x);
  transmitter::reserve(data::magnetometer_y);
  transmitter::reserve(data::magnetometer_z);

  transmitter::reserve(data::gyroscope_x);
  transmitter::reserve(data::gyroscope_y);
  transmitter::reserve(data::gyroscope_z);

  transmitter::reserve(data::orientation_x);
  transmitter::reserve(data::orientation_y);
  transmitter::reserve(data::orientation_z);

  transmitter::reserve(data::linear_acceleration_x);
  transmitter::reserve(data::linear_acceleration_y);
  transmitter::reserve(data::linear_acceleration_z);

  transmitter::send();
}


void transmitter::reserve(float value) {
  transmitter::converter.value = value;
  transmitter::downlinkData[transmitter::offset + 0] = transmitter::converter.data[0];
  transmitter::downlinkData[transmitter::offset + 1] = transmitter::converter.data[1];
  transmitter::downlinkData[transmitter::offset + 2] = transmitter::converter.data[2];
  transmitter::downlinkData[transmitter::offset + 3] = transmitter::converter.data[3];
  transmitter::offset += 4;
}


void transmitter::send() {
  if (LoRa.beginPacket()) {
    LoRa.write(transmitter::downlinkData, transmitter::offset);
    LoRa.endPacket(true);
    transmitter::offset = 0;
  }
}