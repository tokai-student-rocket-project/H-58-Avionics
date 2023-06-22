#include <TaskManager.h>
#include "CANSTM.hpp"
#include "PullupPin.hpp"
#include "OutputPin.hpp"
#include "DetectionCounter.hpp"
#include "Shiranui.hpp"
#include "AnalogVoltage.hpp"


namespace flightMode {
  enum class Mode : uint8_t {
    SLEEP,
    STANDBY,
    THRUST,
    CLIMB,
    DESCENT,
    DECEL,
    PARACHUTE,
    LAND,
    SHUTDOWN
  };

  Mode activeMode;

  void changeMode(flightMode::Mode nextMode);
}

namespace timer {
  uint32_t thrust_time = 3000;
  uint32_t apogee_time = 10000;
  uint32_t first_separation_time = 11000;
  uint32_t second_separation_time = 15000;
  uint32_t land_time = 25000;
  uint32_t shutdown_time = 26000;

  uint32_t referenceTime;

  void setReferenceTime();
  bool isElapsedTime(uint32_t time);

  void task10Hz();
  void task100Hz();
}

namespace sensor {
  AnalogVoltage supply(A7);
  AnalogVoltage battery(A2);
  AnalogVoltage pool(A3);

  PullupPin flightPin(D11);
  DetectionCounter liftoffDetector(3);
  DetectionCounter resetDetector(10);
}

namespace indicator {
  OutputPin canSend(D0);
  OutputPin canReceive(D1);

  OutputPin flightModeBit0(D8);
  OutputPin flightModeBit1(D7);
  OutputPin flightModeBit2(D6);
  OutputPin flightModeBit3(D3);

  void indicateFlightMode(flightMode::Mode mode);
}

namespace control {
  OutputPin camera(D9);
  Shiranui sn3(A0, "drogue");
  Shiranui sn4(D13, "main");
}

namespace connection {
  CANSTM can;
}

namespace data {
  float altitude;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;

  float voltage_supply, voltage_battery, voltage_pool;
}

namespace develop {
  PullupPin debugMode(D12);

  bool isDebugMode;
}


void setup() {
  // 起動モードの判定
  develop::isDebugMode = !develop::debugMode.isOpen();

  // デバッグ用シリアルポートの準備
  if (develop::isDebugMode) {
    Serial.begin(115200);
    delay(800);
  }

  // デバッグ中はピンが干渉するので電圧監視を行わない
  if (!develop::isDebugMode) {
    analogReadResolution(12);
    sensor::supply.begin(3300, 750);
    sensor::battery.begin(4700, 820);
    sensor::pool.begin(5600, 820);
  }

  connection::can.begin();

  flightMode::changeMode(flightMode::Mode::SLEEP);

  Tasks.add(timer::task10Hz)->startFps(10);
  Tasks.add(timer::task100Hz)->startFps(100);
}


void loop() {
  Tasks.update();

  if (connection::can.available()) {
    switch (connection::can.getLatestMessageLabel()) {
    case CANSTM::Label::ALTITUDE:
      connection::can.receiveScalar(&data::altitude);
      break;
    case CANSTM::Label::LINEAR_ACCELERATION:
      connection::can.receiveVector(&data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
      break;
    }

    indicator::canReceive.toggle();
  }
}


void flightMode::changeMode(flightMode::Mode nextMode) {
  if (flightMode::activeMode == nextMode) return;

  switch (nextMode) {
  case (flightMode::Mode::SLEEP):
    control::camera.off();
    break;

  case (flightMode::Mode::STANDBY):
    control::camera.on();
    break;

  case (flightMode::Mode::THRUST):
    control::camera.on();
    timer::setReferenceTime();
    break;

  case (flightMode::Mode::CLIMB):
    break;

  case (flightMode::Mode::DESCENT):
    break;

  case (flightMode::Mode::DECEL):
    control::sn3.separate();
    break;

  case (flightMode::Mode::PARACHUTE):
    control::sn4.separate();
    break;

  case (flightMode::Mode::LAND):
    break;

  case (flightMode::Mode::SHUTDOWN):
    control::camera.off();
    break;
  }

  flightMode::activeMode = nextMode;

  indicator::indicateFlightMode(flightMode::activeMode);
}


void timer::setReferenceTime() {
  timer::referenceTime = millis();
}


bool timer::isElapsedTime(uint32_t time) {
  return (millis() - timer::referenceTime) >= time;
}


void timer::task10Hz() {
  connection::can.sendStatus(CANSTM::Label::STATUS,
    static_cast<uint8_t>(flightMode::activeMode),
    control::camera.get(),
    control::sn3.get(),
    control::sn4.get()
  );

  // デバッグ中はピンが干渉するので電圧監視を行わない
  if (!develop::isDebugMode) {
    data::voltage_supply = sensor::supply.voltage();
    data::voltage_battery = sensor::battery.voltage();
    data::voltage_pool = sensor::pool.voltage();
  }

  connection::can.sendScalar(CANSTM::Label::VOLTAGE_SUPPLY, data::voltage_supply);
  connection::can.sendScalar(CANSTM::Label::VOLTAGE_BATTERY, data::voltage_battery);
  connection::can.sendScalar(CANSTM::Label::VOLTAGE_POOL, data::voltage_pool);
  indicator::canSend.toggle();
}


void timer::task100Hz() {
  sensor::liftoffDetector.update(sensor::flightPin.isOpen());
  sensor::resetDetector.update(!sensor::flightPin.isOpen());

  if (sensor::liftoffDetector.isDetected() && (flightMode::activeMode == flightMode::Mode::SLEEP || flightMode::activeMode == flightMode::Mode::STANDBY)) {
    flightMode::changeMode(flightMode::Mode::THRUST);
  }

  if (sensor::resetDetector.isDetected()) {
    flightMode::changeMode(flightMode::Mode::SLEEP);
  }

  switch (flightMode::activeMode) {
  case (flightMode::Mode::SLEEP):
    if (false) {
      flightMode::changeMode(flightMode::Mode::STANDBY);
    }
    break;

  case (flightMode::Mode::STANDBY):
    if (false) {
      flightMode::changeMode(flightMode::Mode::THRUST);
    }
    break;

  case (flightMode::Mode::THRUST):
    if (timer::isElapsedTime(timer::thrust_time)) {
      flightMode::changeMode(flightMode::Mode::CLIMB);
    }
    break;

  case (flightMode::Mode::CLIMB):
    if (timer::isElapsedTime(timer::apogee_time)) {
      flightMode::changeMode(flightMode::Mode::DESCENT);
    }
    break;

  case (flightMode::Mode::DESCENT):
    if (timer::isElapsedTime(timer::first_separation_time)) {
      flightMode::changeMode(flightMode::Mode::DECEL);
    }
    break;

  case (flightMode::Mode::DECEL):
    if (timer::isElapsedTime(timer::second_separation_time)) {
      flightMode::changeMode(flightMode::Mode::PARACHUTE);
    }
    break;

  case (flightMode::Mode::PARACHUTE):
    if (timer::isElapsedTime(timer::land_time)) {
      flightMode::changeMode(flightMode::Mode::LAND);
    }
    break;

  case (flightMode::Mode::LAND):
    if (timer::isElapsedTime(timer::shutdown_time)) {
      flightMode::changeMode(flightMode::Mode::SHUTDOWN);
    }
    break;

  case (flightMode::Mode::SHUTDOWN):
    if (false) {
      flightMode::changeMode(flightMode::Mode::SLEEP);
    }
    break;
  }
}


void indicator::indicateFlightMode(flightMode::Mode mode) {
  uint8_t modeNumber = static_cast<uint8_t>(mode);
  indicator::flightModeBit0.set(modeNumber & (1 << 0));
  indicator::flightModeBit1.set(modeNumber & (1 << 1));
  indicator::flightModeBit2.set(modeNumber & (1 << 2));
  indicator::flightModeBit3.set(modeNumber & (1 << 3));
}