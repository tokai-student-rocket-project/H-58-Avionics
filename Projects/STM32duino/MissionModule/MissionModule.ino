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


namespace timer {
  uint32_t referenceTime;

  void task50Hz();
  void task1kHz();
}

namespace scheduler {
  bool doSensing = false;

  uint32_t writePosition = 0;
  uint32_t readPosition = 0;
}

namespace sensor {
  ADXL375 adxl(15);
}

namespace recorder {
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

  void handleSystemStatus();
}

namespace data {
  float acceleration_x, acceleration_y, acceleration_z;
}


void setup() {
  // デバッグ用シリアルポートの準備
  Serial.begin(115200);
  while (!Serial);
  delay(800);

  // FRAMとSDの電源は常にON
  control::recorderPower.on();

  LoRa.begin(925.8E6);
  LoRa.setSignalBandwidth(500E3);

  SPI.begin();

  sensor::adxl.begin();

  connection::can.begin();

  connection::can.sendEvent(CANMCP::Publisher::MISSION_MODULE, CANMCP::EventCode::SETUP);

  Tasks.add(timer::task50Hz)->startFps(50);
  Tasks.add(timer::task1kHz)->startFps(1000);
}


void loop() {
  Tasks.update();

  //CAN受信処理
  if (connection::can.available()) {
    switch (connection::can.getLatestLabel()) {
    case CANMCP::Label::SYSTEM_STATUS:
      connection::handleSystemStatus();
      indicator::canReceive.toggle();
      break;
    }
  }
}


void timer::task50Hz() {
  // 計測中はダウンリンクを送信しない
  if (scheduler::doSensing) {
    return;
  }

  // 全て送信し終えているなら何もしない
  if (scheduler::readPosition >= scheduler::writePosition) {
    return;
  }

  // 本当は可変長
  uint8_t data[19];

  LoRa.beginPacket();
  LoRa.write(data, 19);
  LoRa.endPacket();
  indicator::loRaSend.toggle();

  // 50Hz分に間引く
  scheduler::readPosition += 380;
}


void timer::task1kHz() {
  sensor::adxl.getAcceleration(&data::acceleration_x, &data::acceleration_y, &data::acceleration_z);

  if (scheduler::doSensing) {
    const auto& packet = MsgPacketizer::encode(
      0xAA,
      data::acceleration_x, data::acceleration_y, data::acceleration_z
    );

    const uint8_t* data = packet.data.data();
    const uint32_t size = packet.data.size();

    scheduler::writePosition += size;
  }

  Serial.print(millis() / 1000.0, 3);
  Serial.print(" ");
  Serial.print(scheduler::writePosition);
  Serial.print(" ");
  Serial.println(scheduler::readPosition);
}


void connection::handleSystemStatus() {
  uint8_t flightMode;
  bool cameraState, sn3State, doLogging;

  connection::can.receiveSystemStatus(&flightMode, &cameraState, &sn3State, &doLogging);

  // フライトモードがSTANDBYかTHRUSTなら加速度の計測を行う
  scheduler::doSensing = (flightMode == 1 || flightMode == 2);
}