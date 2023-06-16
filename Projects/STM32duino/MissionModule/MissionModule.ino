#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <TaskManager.h>
#include <mcp2515_can.h>
#include "PullupPin.hpp"
#include "OutputPin.hpp"
#include "ADXL375.hpp"
#include "Sd.hpp"


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

  mcp2515_can can(7);

  void initialize();
  void receiveStatus(uint8_t* data, uint8_t* mode, uint8_t* camera, uint8_t* separatorDrogue, uint8_t* separatorMain);
}

namespace timer {
  uint32_t referenceTime;

  void task50Hz();
  void task1k2Hz();

  void invalidSdBlink();
}

namespace sensor {
  ADXL375 adxl;
}

namespace recorder {
  Sd sd(6);

  PullupPin cardDetection(3);

  bool doRecording;
}

namespace indicator {
  OutputPin canReceive(0);

  OutputPin loRaSend(A1);

  OutputPin sdStatus(4);
  OutputPin recorderStatus(2);
}

namespace control {
  OutputPin recorderPower(5);
}

namespace data {
  uint8_t mode;
  uint8_t camera;
  uint8_t separatorDrogue;
  uint8_t separatorMain;

  float acceleration_x, acceleration_y, acceleration_z;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(925.8E6);
  LoRa.setSignalBandwidth(500E3);

  // 開発中: 保存は常に行う表示
  control::recorderPower.on();
  indicator::recorderStatus.on();

  SPI.begin();

  if (recorder::sd.begin()) {
    indicator::sdStatus.on();
  }
  else {
    Tasks.add("invalidSdBlink", timer::invalidSdBlink)->startFps(2);
  }

  Wire.begin();
  Wire.setClock(400000);

  sensor::adxl.begin();

  canbus::initialize();

  Tasks.add(timer::task50Hz)->startFps(50);
  Tasks.add(timer::task1k2Hz)->startFps(1200);
}


void loop() {
  Tasks.update();

  // SDの検知の更新
  // SDを新しく検知した時
  if (!recorder::sd.isRunning() && !recorder::cardDetection.isOpen()) {
    recorder::sd.begin();
    Tasks.erase("invalidSdBlink");
    indicator::sdStatus.on();
  }

  // SDが検知できなくなった時
  if (recorder::sd.isRunning() && recorder::cardDetection.isOpen()) {
    recorder::sd.end();
    Tasks.add("invalidSdBlink", timer::invalidSdBlink)->startFps(2);
  }

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
  canbus::can.begin(CAN_500KBPS, MCP_8MHz);
}


void canbus::receiveStatus(uint8_t* data, uint8_t* mode, uint8_t* camera, uint8_t* separatorDrogue, uint8_t* separatorMain) {
  *mode = data[0];
  *camera = data[1];
  *separatorDrogue = data[2];
  *separatorMain = data[3];

  indicator::canReceive.toggle();
}


void timer::task50Hz() {
  const auto& packet = MsgPacketizer::encode(
    0x00,
    data::acceleration_x,
    data::acceleration_y,
    data::acceleration_z
  );

  LoRa.beginPacket();
  LoRa.write(packet.data.data(), packet.data.size());
  LoRa.endPacket();
  indicator::loRaSend.toggle();
}


void timer::task1k2Hz() {
  sensor::adxl.getAcceleration(&data::acceleration_x, &data::acceleration_y, &data::acceleration_z);

  Serial.print("x:"); Serial.print(data::acceleration_x / 9.8);
  Serial.print(",y:"); Serial.print(data::acceleration_y / 9.8);
  Serial.print(",z:"); Serial.println(data::acceleration_z / 9.8);
}


void timer::invalidSdBlink() {
  indicator::sdStatus.toggle();
}