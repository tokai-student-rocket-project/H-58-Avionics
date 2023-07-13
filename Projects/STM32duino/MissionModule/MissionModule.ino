#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <TaskManager.h>
#include "CANMCP.hpp"
#include "Switch.hpp"
#include "LED.hpp"
#include "Blinker.hpp"
#include "ADXL375.hpp"
#include "Sd.hpp"
#include "Var.hpp"


namespace timer {
  uint32_t referenceTime;

  void task50Hz();
  void task1k2Hz();
}

namespace sensor {
  ADXL375 adxl(15);
}

namespace recorder {
  Sd sd(6);

  Switch cardDetection(3);

  bool doRecording;
}

namespace indicator {
  LED canReceive(0);

  LED loRaSend(A1);

  Blinker sdStatus(4, "invalidSd");
  LED recorderStatus(2);
}

namespace control {
  LED recorderPower(5);
}

namespace connection {
  CANMCP can(7);
}

namespace data {
  Var::FlightMode mode;
  Var::State camera, sn3;
  bool doLogging;

  float acceleration_x, acceleration_y, acceleration_z;
}


void setup() {
  Serial.begin(115200);
  delay(800);

  LoRa.begin(925.8E6);
  LoRa.setSignalBandwidth(500E3);

  // 開発中: 保存は常に行う表示
  control::recorderPower.on();
  indicator::recorderStatus.on();

  SPI.begin();

  sensor::adxl.begin();

  if (recorder::sd.begin()) {
    indicator::sdStatus.on();
  }
  else {
    indicator::sdStatus.startBlink(2);
  }

  connection::can.begin();

  connection::can.sendEvent(CANMCP::Publisher::MISSION_MODULE, CANMCP::EventCode::SETUP);

  Tasks.add(timer::task50Hz)->startFps(50);
  Tasks.add(timer::task1k2Hz)->startFps(1200);
}


void loop() {
  Tasks.update();

  // SDの検知の更新
  // SDを新しく検知した時
  if (!recorder::sd.isRunning() && recorder::cardDetection.is(Var::SwitchState::CLOSE)) {
    recorder::sd.begin();
    indicator::sdStatus.stopBlink();
    indicator::sdStatus.on();
  }

  // SDが検知できなくなった時
  if (recorder::sd.isRunning() && recorder::cardDetection.is(Var::SwitchState::OPEN)) {
    recorder::sd.end();
    indicator::sdStatus.startBlink(2);
  }

  //CAN受信処理
  if (connection::can.available()) {
    switch (connection::can.getLatestLabel()) {
    case CANMCP::Label::SYSTEM_STATUS:
      connection::can.receiveSystemStatus(&data::mode, &data::camera, &data::sn3, &data::doLogging);
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

  Serial.print(data::acceleration_x);
  Serial.print(",");
  Serial.print(data::acceleration_y);
  Serial.print(",");
  Serial.println(data::acceleration_z);
}