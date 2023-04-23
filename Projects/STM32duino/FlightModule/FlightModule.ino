#include <ACAN_STM32.h>
#include <TaskManager.h>
#include "PullupPin.hpp"
#include "OutputPin.hpp"
#include "Debugger.hpp"


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

  void initialize();
  void sendStatus(canbus::Id id, uint8_t mode);
  void receiveScalar(CANMessage message, float* value);
  void receiveVector(CANMessage message, float* x, float* y, float* z);
}

namespace flightMode {
  enum class Mode: uint8_t {
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
  OutputPin ledCanReceive(LED_BUILTIN);
  OutputPin ledFlightModeBit0(D5);
  OutputPin ledFlightModeBit1(D6);
  OutputPin ledFlightModeBit2(D7);
  OutputPin ledFlightModeBit3(D8);

  void indicateFlightMode(flightMode::Mode mode);
}

namespace connection {
  OutputPin camera(D9);
  OutputPin separatorDrogue(A1);
  OutputPin separatorMain(A0);
}

namespace data {
  float altitude;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
}

namespace develop {
  Debugger debugger;
  PullupPin wakeUpButton(D3);
  PullupPin ignitionButton(D4);
}


void setup() {
  develop::debugger.initialize();
  canbus::initialize();

  flightMode::changeMode(flightMode::Mode::SLEEP);
  develop::debugger.printMessage("BEGIN");

  Tasks.add(timer::task10Hz)->startIntervalMsec(100);
  Tasks.add(timer::task100Hz)->startIntervalMsec(10);

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

    indicator::ledCanReceive.toggle();
  }
}


void canbus::initialize() {
  ACAN_STM32_Settings settings(1000000);
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);
}


void canbus::sendStatus(canbus::Id id, uint8_t mode) {
  CANMessage message;
  message.id = static_cast<uint32_t>(id);
  message.len = 1;

  message.data[0] = mode;

  can.tryToSendReturnStatus(message);
}


void canbus::receiveScalar(CANMessage message, float* value) {
  canbus::converter.data[0] = message.data[0];
  canbus::converter.data[1] = message.data[1];
  canbus::converter.data[2] = message.data[2];
  canbus::converter.data[3] = message.data[3];
  *value = canbus::converter.value;
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
}


void flightMode::changeMode(flightMode::Mode nextMode) {
  if (flightMode::activeMode == nextMode) return;

  flightMode::activeMode = nextMode;
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
  switch (flightMode::activeMode) {
  case (flightMode::Mode::SLEEP):
    if (!develop::wakeUpButton.isOpen()) {
      flightMode::changeMode(flightMode::Mode::STANDBY);
      develop::debugger.printMessage("WAKE_UP");

      connection::camera.on();
    }
    break;

  case (flightMode::Mode::STANDBY):
    if (!develop::ignitionButton.isOpen()) {
      flightMode::changeMode(flightMode::Mode::THRUST);
      develop::debugger.printMessage("IGNITION");

      timer::setReferenceTime();
    }
    break;

  case (flightMode::Mode::THRUST):
    if (timer::isElapsedTime(timer::thrust_time)) {
      flightMode::changeMode(flightMode::Mode::CLIMB);
      develop::debugger.printMessage("BURNOUT");
    }
    break;

  case (flightMode::Mode::CLIMB):
    if (timer::isElapsedTime(timer::apogee_time)) {
      flightMode::changeMode(flightMode::Mode::DESCENT);
      develop::debugger.printMessage("APOGEE");
    }
    break;

  case (flightMode::Mode::DESCENT):
    if (timer::isElapsedTime(timer::first_separation_time)) {
      flightMode::changeMode(flightMode::Mode::DECEL);
      develop::debugger.printMessage("1ST_SEPARATION");

      connection::separatorDrogue.on();
      Tasks["SeparatorDrogueAutoOff"]->startOnceAfterMsec(1000);
    }
    break;

  case (flightMode::Mode::DECEL):
    if (timer::isElapsedTime(timer::second_separation_time)) {
      flightMode::changeMode(flightMode::Mode::PARACHUTE);
      develop::debugger.printMessage("2ND_SEPARATION");

      connection::separatorMain.on();
      Tasks["SeparatorMainAutoOff"]->startOnceAfterMsec(1000);
    }
    break;

  case (flightMode::Mode::PARACHUTE):
    if (timer::isElapsedTime(timer::land_time)) {
      flightMode::changeMode(flightMode::Mode::LAND);
      develop::debugger.printMessage("LAND");
    }
    break;

  case (flightMode::Mode::LAND):
    if (timer::isElapsedTime(timer::shutdown_time)) {
      flightMode::changeMode(flightMode::Mode::SHUTDOWN);
      develop::debugger.printMessage("SHUTDOWN");

      connection::camera.off();
    }
    break;

  case (flightMode::Mode::SHUTDOWN):
    if (!develop::ignitionButton.isOpen()) {
      flightMode::changeMode(flightMode::Mode::SLEEP);
      develop::debugger.printMessage("RESET");
    }
    break;
  }

  indicator::indicateFlightMode(flightMode::activeMode);
}


void timer::taskSeparatorDrogueAutoOff() {
  connection::separatorDrogue.off();
}


void timer::taskSeparatorMainAutoOff() {
  connection::separatorMain.off();
}


void indicator::indicateFlightMode(flightMode::Mode mode) {
  uint8_t modeNumber = static_cast<uint8_t>(mode);
  indicator::ledFlightModeBit0.set(modeNumber & (1 << 0));
  indicator::ledFlightModeBit1.set(modeNumber & (1 << 1));
  indicator::ledFlightModeBit2.set(modeNumber & (1 << 2));
  indicator::ledFlightModeBit3.set(modeNumber & (1 << 3));
}