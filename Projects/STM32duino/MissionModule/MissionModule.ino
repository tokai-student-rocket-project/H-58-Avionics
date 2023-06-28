#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <TaskManager.h>
#include "CANMCP.hpp"
#include "PullupPin.hpp"
#include "OutputPin.hpp"
#include "Blinker.hpp"
#include "ADXL375.hpp"
#include "Sd.hpp"


namespace timer {
  uint32_t referenceTime;

  void task50Hz();
  void task1k2Hz();
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

  Blinker sdStatus(4, "invalidSd");
  OutputPin recorderStatus(2);
}

namespace control {
  OutputPin recorderPower(5);
}

namespace connection {
  CANMCP can(7);
}

namespace data {
  uint8_t mode;
  bool camera, sn3;

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
    indicator::sdStatus.startBlink(2);
  }

  Wire.begin();
  Wire.setClock(400000);

  sensor::adxl.begin();

  connection::can.begin();

  connection::can.sendEvent(CANMCP::Publisher::MISSION_MODULE, CANMCP::EventCode::SETUP);

  Tasks.add(timer::task50Hz)->startFps(50);
  Tasks.add(timer::task1k2Hz)->startFps(1200);
}


void loop() {
  Tasks.update();

  // SDの検知の更新
  // SDを新しく検知した時
  if (!recorder::sd.isRunning() && !recorder::cardDetection.isOpen()) {
    recorder::sd.begin();
    indicator::sdStatus.stopBlink();
    indicator::sdStatus.on();
  }

  // SDが検知できなくなった時
  if (recorder::sd.isRunning() && recorder::cardDetection.isOpen()) {
    recorder::sd.end();
    indicator::sdStatus.startBlink(2);
  }

  //CAN受信処理
  if (connection::can.available()) {
    switch (connection::can.getLatestLabel()) {
    case CANMCP::Label::SYSTEM_STATUS:
      connection::can.receiveStatus(&data::mode, &data::camera, &data::sn3);
      indicator::canReceive.toggle();
      break;
    }
  }
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
}