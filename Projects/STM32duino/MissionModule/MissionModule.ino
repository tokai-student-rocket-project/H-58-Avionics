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
#include "Logger.hpp"
#include "Sender.hpp"


namespace timer {
  void sendStatusTask();
  void sendDataTask();
  void measurementTask();

  uint32_t referenceTime;
  float dataRate;
}

namespace scheduler {
  Var::FlightMode flightModePrevious;

  Logger logger(A6, A5);
  Sender sender(A6, A5);
  bool doLogging = false;
  bool doSend = false;

  uint32_t count;
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
  // while (!Serial);
  // delay(800);

  // FRAMとSDの電源は常にON
  control::recorderPower.on();

  LoRa.begin(925.8E6);
  LoRa.setSignalBandwidth(500E3);

  SPI.begin();

  sensor::adxl.begin();

  connection::can.begin();
  connection::can.sendEvent(CANMCP::Publisher::MISSION_MODULE, CANMCP::EventCode::SETUP);

  Tasks.add(timer::sendStatusTask)->startFps(19);
  Tasks.add(timer::sendDataTask)->startFps(50);
  Tasks.add(timer::measurementTask)->startFps(1200);

  Tasks.add("stop-logging", [&]() {
    scheduler::doLogging = false;
    scheduler::doSend = true;
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


void timer::sendStatusTask() {
  // CANにデータを流す
  connection::can.sendMissionStatus(
    static_cast<uint8_t>(scheduler::logger.getUsage())
    // ,timer::dataRate
  );

  // TODO 送信進捗送信

  indicator::canSend.toggle();

  // Serial.println(loggerUsage);
  // Serial.println(timer::dataRate);

  // Serial.print(scheduler::logger.getOffset());
  // Serial.print("\t");
  // Serial.print(scheduler::sender.getOffset());
  // Serial.print("\n");
}


void timer::sendDataTask() {
  if (!scheduler::doSend) return;

  // 送信し切ったら終了
  if (scheduler::sender.getOffset() > scheduler::logger.getOffset()) {
    scheduler::doSend = false;
    indicator::loRaSend.off();
    return;
  }

  scheduler::sender.send(scheduler::count);
  indicator::loRaSend.toggle();
  scheduler::count++;
}


void timer::measurementTask() {
  float x, y, z;
  sensor::adxl.getAcceleration(&x, &y, &z);

  if (scheduler::doLogging) {
    scheduler::logger.log(
      millis(), static_cast<uint8_t>(scheduler::flightModePrevious),
      x, y, z);
  }

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
    if (!scheduler::doLogging) {
      scheduler::logger.reset();
      scheduler::sender.reset();
      scheduler::doSend = false;
      scheduler::doLogging = true;
      scheduler::count = 0;
    }

    // 3秒間計測
    if (flightMode == Var::FlightMode::THRUST) {
      Tasks["stop-logging"]->startOnceAfterSec(3.0);
    }
  }

  // 開傘時計測
  if (flightMode == Var::FlightMode::PARACHUTE) {
    if (!scheduler::doLogging) {
      scheduler::doSend = false;
      scheduler::doLogging = true;
    }

    // 6秒間計測
    Tasks["stop-logging"]->startOnceAfterSec(6.0);
  }

  scheduler::flightModePrevious = flightMode;
}