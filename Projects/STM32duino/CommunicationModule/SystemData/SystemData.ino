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
  void receiveStatus(uint8_t* data, uint8_t* mode, uint8_t* camera, uint8_t* separatorDrogue, uint8_t* separatorMain);
}

namespace timer {
  void task10Hz();
}

namespace indicator {
  OutputPin ledCanReceive(LED_BUILTIN);
}

namespace transmitter {
  uint8_t downlinkData[256];
  uint8_t offset = 0;

  void reserveState(uint8_t state);
  void send();
}

namespace data {
  uint8_t mode;
  uint8_t camera;
  uint8_t separatorDrogue;
  uint8_t separatorMain;
}


void setup() {
  Serial.begin(115200);
  LoRa.begin(921.8E6);

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

    indicator::ledCanReceive.toggle();
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
}


void timer::task10Hz() {
  transmitter::reserveState(data::mode);
  transmitter::reserveState(data::camera);
  transmitter::reserveState(data::separatorDrogue);
  transmitter::reserveState(data::separatorMain);

  transmitter::send();
}


void transmitter::reserveState(uint8_t state) {
  transmitter::downlinkData[transmitter::offset] = state;
  transmitter::offset += 1;
}


void transmitter::send() {
  if (LoRa.beginPacket()) {
    LoRa.write(transmitter::downlinkData, transmitter::offset);
    LoRa.endPacket(true);
    transmitter::offset = 0;
  }
}