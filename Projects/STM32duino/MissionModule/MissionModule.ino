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
  void task20Hz();
  void task50Hz();
  void task1kHz();

  uint32_t referenceTime;
  float dataRate;
}

namespace scheduler {
  Var::FlightMode flightModePrevious;

  uint32_t thrustOffset = 0;
  uint32_t thrustSize = 0;
  uint32_t openOffset = 0;
  uint32_t openSize = 0;

  // 保存
  bool doThrustLogging = false;
  bool doOpenLogging = false;
  uint32_t writeOffset = 0;

  // 送信
  bool doSendThrust = false;
  bool doSendOpen = false;
  uint32_t readOffset = 0;
}

namespace sensor {
  ADXL375 adxl(15);
}

namespace indicator {
  LED canSend(1);
  LED canReceive(0);
  LED loRaSend(A1);

  LED recorderStatus(2);
}

namespace control {
  LED recorderPower(5);
}

namespace connection {
  CANMCP can(7);

  void handleSystemStatus();
}


void setup() {
  // デバッグ用シリアルポートの準備
  Serial.begin(115200);

  // FRAMとSDの電源は常にON
  control::recorderPower.on();

  LoRa.begin(925.8E6);
  LoRa.setSignalBandwidth(500E3);

  SPI.begin();

  sensor::adxl.begin();

  connection::can.begin();
  connection::can.sendEvent(CANMCP::Publisher::MISSION_MODULE, CANMCP::EventCode::SETUP);

  Tasks.add(timer::task20Hz)->startFps(19);
  Tasks.add(timer::task50Hz)->startFps(50);
  Tasks.add(timer::task1kHz)->startFps(1200);

  Tasks.add("stop-thrust-logging", [&]() {
    scheduler::doThrustLogging = false;
    scheduler::doSendThrust = true;
    });
  Tasks.add("stop-open-logging", [&]() {
    scheduler::doOpenLogging = false;
    scheduler::doSendOpen = true;
    });
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


void timer::task20Hz() {
  float loggerUsage = ((float)scheduler::writeOffset / (float)(262144 * 2)) * 100.0;
  // CANにデータを流す
  connection::can.sendMissionStatus(
    static_cast<uint8_t>(loggerUsage)
    // ,timer::dataRate
  );

  indicator::canSend.toggle();

  // Serial.println(loggerUsage);
  // Serial.println(timer::dataRate);
}


void timer::task50Hz() {
  // TODO FRAMから読み出し
  uint32_t readSize = 32 * 20;

  if (scheduler::doSendThrust) {
    // 送信し切ったら終わり
    if ((int32_t)(scheduler::thrustSize - readSize) < 0) {
      scheduler::doSendThrust = false;
      return;
    }

    // TODO 送信

    scheduler::thrustSize -= readSize;
  }

  if (scheduler::doSendOpen) {
    // 送信し切ったら終わり
    if ((int32_t)(scheduler::openSize - readSize) < 0) {
      scheduler::doSendOpen = false;
      return;
    }

    // TODO 送信

    scheduler::openSize -= readSize;
  }
}


void timer::task1kHz() {
  float x, y, z;
  sensor::adxl.getAcceleration(&x, &y, &z);

  // 燃焼時計測
  if (scheduler::doThrustLogging) {
    const auto& packet = MsgPacketizer::encode(0xAA, x, y, z);
    const uint8_t* data = packet.data.data();
    const uint32_t size = packet.data.size();

    scheduler::writeOffset += size;
    scheduler::thrustSize += size;

    // TODO FRAMへ保存
  }

  // 開傘時計測
  if (scheduler::doOpenLogging) {
    const auto& packet = MsgPacketizer::encode(0xAA, x, y, z);
    const uint8_t* data = packet.data.data();
    const uint32_t size = packet.data.size();

    scheduler::writeOffset += size;
    scheduler::openSize += size;

    // TODO FRAMへ保存
  }

  Serial.print(scheduler::thrustOffset);
  Serial.print("\t");
  Serial.print(scheduler::thrustSize);
  Serial.print("\t");
  Serial.print(scheduler::openOffset);
  Serial.print("\t");
  Serial.print(scheduler::openSize);
  Serial.println();

  // ODR情報
  // 1kHzを超えるとmillis()では見れない
  uint32_t time = micros();
  timer::dataRate = 1000000.0 / (float)(time - timer::referenceTime);
  timer::referenceTime = time;
}


void connection::handleSystemStatus() {
  Var::FlightMode flightMode;
  Var::State camera, sn3;
  bool doLogging;
  uint16_t flightTime;
  uint8_t loggerUsage;

  connection::can.receiveSystemStatus(&flightMode, &camera, &sn3, &doLogging, &flightTime, &loggerUsage);

  // フライトモードが切り替わった時のみ判定したい
  if (flightMode == scheduler::flightModePrevious) return;

  // 燃焼時計測
  if (flightMode == Var::FlightMode::STANDBY || flightMode == Var::FlightMode::THRUST) {
    if (!scheduler::doThrustLogging) {
      // 書き込み位置のリセット
      scheduler::writeOffset = 0;
      scheduler::thrustOffset = 0;
      scheduler::thrustSize = 0;
      scheduler::openOffset = 0;
      scheduler::openSize = 0;

      scheduler::thrustOffset = scheduler::writeOffset;
      scheduler::doThrustLogging = true;
    }

    // 3秒間計測
    if (flightMode == Var::FlightMode::THRUST) {
      Tasks["stop-thrust-logging"]->startOnceAfterSec(3.0);
    }
  }

  // 開傘時計測
  if (flightMode == Var::FlightMode::PARACHUTE) {
    if (!scheduler::doOpenLogging) {
      scheduler::openOffset = scheduler::writeOffset;
      scheduler::doOpenLogging = true;
    }

    // 6秒間計測
    Tasks["stop-open-logging"]->startOnceAfterSec(6.0);
  }

  scheduler::flightModePrevious = flightMode;
}