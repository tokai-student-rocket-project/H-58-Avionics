#include <ACAN_STM32.h>
#include <TaskManager.h>
#include "PullupPin.hpp"
#include "OutputPin.hpp"


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

  enum class Axis : uint8_t {
    X,
    Y,
    Z
  };

  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  void initialize();
  void sendStatus(canbus::Id id, uint8_t mode);
  void receiveScalar(CANMessage message, float* value);
  void receiveVector(CANMessage message, float* x, float* y, float* z);
}

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
  void taskSeparatorDrogueAutoOff();
  void taskSeparatorMainAutoOff();
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

namespace connection {
  OutputPin camera(D9);
  OutputPin sn3(A0);
  OutputPin sn4(D13);

  PullupPin flightPin(D11);
}

namespace data {
  float altitude;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
}


void setup() {
  canbus::initialize();

  flightMode::changeMode(flightMode::Mode::SLEEP);

  Tasks.add(timer::task10Hz)->startFps(10);
  Tasks.add(timer::task100Hz)->startFps(100);

  Tasks.add("SeparatorDrogueAutoOff", timer::taskSeparatorDrogueAutoOff);
  Tasks.add("SeparatorMainAutoOff", timer::taskSeparatorMainAutoOff);
}


void loop() {
  Tasks.update();

  if (can.available0()) {
    CANMessage message;
    can.receive0(message);

    switch (message.id) {
    case static_cast<uint32_t>(canbus::Id::ALTITUDE):
      canbus::receiveScalar(message, &data::altitude);
      break;
    case static_cast<uint32_t>(canbus::Id::LINEAR_ACCELERATION):
      canbus::receiveVector(message, &data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
      break;
    }
  }
}


void canbus::initialize() {
  ACAN_STM32_Settings settings(500000);
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);
}


void canbus::sendStatus(canbus::Id id, uint8_t mode) {
  CANMessage message;
  message.id = static_cast<uint32_t>(id);
  message.len = 4;

  message.data[0] = mode;
  message.data[1] = connection::camera.get();
  message.data[2] = connection::sn3.get();
  message.data[3] = connection::sn4.get();;

  can.tryToSendReturnStatus(message);

  indicator::canSend.toggle();
}


void canbus::receiveScalar(CANMessage message, float* value) {
  canbus::converter.data[0] = message.data[0];
  canbus::converter.data[1] = message.data[1];
  canbus::converter.data[2] = message.data[2];
  canbus::converter.data[3] = message.data[3];
  *value = canbus::converter.value;

  indicator::canReceive.toggle();
}


void canbus::receiveVector(CANMessage message, float* x, float* y, float* z) {
  canbus::converter.data[0] = message.data[1];
  canbus::converter.data[1] = message.data[2];
  canbus::converter.data[2] = message.data[3];
  canbus::converter.data[3] = message.data[4];

  switch (message.data[0]) {
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

  indicator::canReceive.toggle();
}


void flightMode::changeMode(flightMode::Mode nextMode) {
  if (flightMode::activeMode == nextMode) return;

  switch (nextMode) {
  case (flightMode::Mode::SLEEP):
    break;

  case (flightMode::Mode::STANDBY):
    connection::camera.on();
    break;

  case (flightMode::Mode::THRUST):
    connection::camera.on();
    timer::setReferenceTime();
    break;

  case (flightMode::Mode::CLIMB):
    break;

  case (flightMode::Mode::DESCENT):
    break;

  case (flightMode::Mode::DECEL):
    connection::sn3.on();
    Tasks["SeparatorDrogueAutoOff"]->startOnceAfterSec(3);
    break;

  case (flightMode::Mode::PARACHUTE):
    connection::sn4.on();
    Tasks["SeparatorMainAutoOff"]->startOnceAfterSec(3);
    break;

  case (flightMode::Mode::LAND):
    break;

  case (flightMode::Mode::SHUTDOWN):
    connection::camera.off();
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
  canbus::sendStatus(canbus::Id::STATUS, static_cast<uint8_t>(flightMode::activeMode));
}


void timer::task100Hz() {
  if (connection::flightPin.isOpen() && (flightMode::activeMode == flightMode::Mode::SLEEP || flightMode::activeMode == flightMode::Mode::STANDBY)) {
    flightMode::changeMode(flightMode::Mode::THRUST);
  }

  if (!connection::flightPin.isOpen()) {
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


void timer::taskSeparatorDrogueAutoOff() {
  connection::sn3.off();
}


void timer::taskSeparatorMainAutoOff() {
  connection::sn4.off();
}


void indicator::indicateFlightMode(flightMode::Mode mode) {
  uint8_t modeNumber = static_cast<uint8_t>(mode);
  indicator::flightModeBit0.set(modeNumber & (1 << 0));
  indicator::flightModeBit1.set(modeNumber & (1 << 1));
  indicator::flightModeBit2.set(modeNumber & (1 << 2));
  indicator::flightModeBit3.set(modeNumber & (1 << 3));
}