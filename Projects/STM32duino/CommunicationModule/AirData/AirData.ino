#include <SPI.h>
#include <LoRa.h>
#include <mcp2515_can.h>
#include "OutputPin.hpp"


namespace canbus {
  enum class Id: uint32_t {
    TEMPERATURE,
    PRESSURE,
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
}

namespace indicator {
  OutputPin ledCanReceive(LED_BUILTIN);
}


void setup() {
  Serial.begin(115200);
  LoRa.begin(923.8E6);

  canbus::initialize();
}


void loop() {
  if (CAN_MSGAVAIL == canbus::can.checkReceive()) {
    uint8_t len = 0;
    uint8_t data[8];

    canbus::can.readMsgBuf(&len, data);
    uint32_t id = canbus::can.getCanId();

    Serial.println(id);

    indicator::ledCanReceive.toggle();
  }
}


void canbus::initialize() {
  canbus::can.begin(CAN_1000KBPS, MCP_16MHz);
}