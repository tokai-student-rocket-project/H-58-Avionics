#include <ACAN_STM32.h>
#include <TaskManager.h>
#include "PullupPin.hpp"


namespace canbus {
  enum class Id {
    TEMPERATURE,
    PRESSURE,
    ACCELERATION,
    GYROSCOPE,
    MAGNETOMETER,
    ORIENTATION,
    LINEAR_ACCELERATION,
    GRAVITY
  };

  enum class Axis {
    X,
    Y,
    Z
  };

  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  void receiveNorm(CANMessage message, float* value);
  void receiveVector(CANMessage message, float* x, float* y, float* z);
}

namespace device {
  PullupPin flightPin(D4);
}

namespace flightMode {
  enum class Mode {
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

  void changeMode(Mode nextMode);
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
}

namespace detector {
  bool canWakeUp();
  bool canIgnition();
}

namespace indicator {
  void indicateFlightMode(flightMode::Mode mode);
}

namespace data {
  float pressure;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
}


void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);

  ACAN_STM32_Settings settings(1000000);
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);

  flightMode::changeMode(flightMode::Mode::SLEEP);

  Tasks.add(task100Hz)->startIntervalMsec(10);
}


void loop() {
  Tasks.update();

  if (can.available0()) {
    CANMessage message;
    can.receive0(message);

    switch (message.id) {
    case static_cast<uint8_t>(canbus::Id::PRESSURE):
      canbus::receiveNorm(message, &data::pressure);
      break;
    case static_cast<uint8_t>(canbus::Id::LINEAR_ACCELERATION):
      canbus::receiveVector(message, &data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
      break;

    default:
      break;
    }

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}


void task100Hz() {
  Serial.print(data::linear_acceleration_x);
  Serial.print(",");
  Serial.print(data::linear_acceleration_y);
  Serial.print(",");
  Serial.println(data::linear_acceleration_z);

  device::flightPin.update();

  switch (flightMode::activeMode) {
  case (flightMode::Mode::SLEEP):
    if (detector::canWakeUp()) {
      flightMode::changeMode(flightMode::Mode::STANDBY);
      // Serial.println("WAKE_UP");
    }
    break;

  case (flightMode::Mode::STANDBY):
    if (detector::canIgnition()) {
      flightMode::changeMode(flightMode::Mode::THRUST);
      // Serial.println("IGNITION");

      timer::setReferenceTime();
    }
    break;

  case (flightMode::Mode::THRUST):
    if (timer::isElapsedTime(timer::thrust_time)) {
      flightMode::changeMode(flightMode::Mode::CLIMB);
      // Serial.println("BURNOUT");
    }
    break;

  case (flightMode::Mode::CLIMB):
    if (timer::isElapsedTime(timer::apogee_time)) {
      flightMode::changeMode(flightMode::Mode::DESCENT);
      // Serial.println("APOGEE");
    }
    break;

  case (flightMode::Mode::DESCENT):
    if (timer::isElapsedTime(timer::first_separation_time)) {
      flightMode::changeMode(flightMode::Mode::DECEL);
      // Serial.println("1ST_SEPARATION");
    }
    break;

  case (flightMode::Mode::DECEL):
    if (timer::isElapsedTime(timer::second_separation_time)) {
      flightMode::changeMode(flightMode::Mode::PARACHUTE);
      // Serial.println("2ND_SEPARATION");
    }
    break;

  case (flightMode::Mode::PARACHUTE):
    if (timer::isElapsedTime(timer::land_time)) {
      flightMode::changeMode(flightMode::Mode::LAND);
      // Serial.println("LAND");
    }
    break;

  case (flightMode::Mode::LAND):
    if (timer::isElapsedTime(timer::shutdown_time)) {
      flightMode::changeMode(flightMode::Mode::SHUTDOWN);
      // Serial.println("SHUTDOWN");
    }
    break;

  default:
    break;
  }

  indicator::indicateFlightMode(flightMode::activeMode);
}


void timer::setReferenceTime() {
  timer::referenceTime = millis();
}


bool timer::isElapsedTime(uint32_t time) {
  return (millis() - timer::referenceTime) >= time;
}


bool detector::canWakeUp() {
  return !device::flightPin.isOpen();
}


bool detector::canIgnition() {
  return !device::flightPin.isOpen();
}


void indicator::indicateFlightMode(flightMode::Mode mode) {
  uint8_t modeNumber = static_cast<uint8_t>(mode);
  digitalWrite(D5, (modeNumber & (1 << 0)));
  digitalWrite(D6, (modeNumber & (1 << 1)));
  digitalWrite(D7, (modeNumber & (1 << 2)));
  digitalWrite(D8, (modeNumber & (1 << 3)));
}


void canbus::receiveNorm(CANMessage message, float* value) {
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

  default:
    break;
  }
}


void flightMode::changeMode(Mode nextMode) {
  if (flightMode::activeMode == nextMode) return;

  flightMode::activeMode = nextMode;
}