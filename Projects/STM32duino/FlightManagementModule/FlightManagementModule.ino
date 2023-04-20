#include <ACAN_STM32.h>
#include <TaskManager.h>
#include "FlightModeManager.hpp"
#include "TimerManager.hpp"
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

namespace manager {
  FlightModeManager flightModeManager;
  TimerManager timerManager;
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

  ACAN_STM32_Settings settings(1000000); // 1 Mbit/s
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);

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

  switch (manager::flightModeManager.getActiveMode()) {
  case (FlightModeManager::FlightMode::SLEEP):
    if (canWakeUp()) {
      manager::flightModeManager.changeMode(FlightModeManager::FlightMode::STANDBY);
      // Serial.println("WAKE_UP");
    }
    break;

  case (FlightModeManager::FlightMode::STANDBY):
    if (canIgnition()) {
      manager::flightModeManager.changeMode(FlightModeManager::FlightMode::THRUST);
      // Serial.println("IGNITION");

      manager::timerManager.setReferenceTime();
    }
    break;

  case (FlightModeManager::FlightMode::THRUST):
    if (manager::timerManager.isElapsedTime(manager::timerManager.ThrustTime)) {
      manager::flightModeManager.changeMode(FlightModeManager::FlightMode::CLIMB);
      // Serial.println("BURNOUT");
    }
    break;

  case (FlightModeManager::FlightMode::CLIMB):
    if (manager::timerManager.isElapsedTime(manager::timerManager.ApogeeTime)) {
      manager::flightModeManager.changeMode(FlightModeManager::FlightMode::DESCENT);
      // Serial.println("APOGEE");
    }
    break;

  case (FlightModeManager::FlightMode::DESCENT):
    if (manager::timerManager.isElapsedTime(manager::timerManager.FirstSeparationTime)) {
      manager::flightModeManager.changeMode(FlightModeManager::FlightMode::DECEL);
      // Serial.println("1ST_SEPARATION");
    }
    break;

  case (FlightModeManager::FlightMode::DECEL):
    if (manager::timerManager.isElapsedTime(manager::timerManager.SecondSeparationTime)) {
      manager::flightModeManager.changeMode(FlightModeManager::FlightMode::PARACHUTE);
      // Serial.println("2ND_SEPARATION");
    }
    break;

  case (FlightModeManager::FlightMode::PARACHUTE):
    if (manager::timerManager.isElapsedTime(manager::timerManager.LandTime)) {
      manager::flightModeManager.changeMode(FlightModeManager::FlightMode::LAND);
      // Serial.println("LAND");
    }
    break;

  case (FlightModeManager::FlightMode::LAND):
    if (manager::timerManager.isElapsedTime(manager::timerManager.ShutdownTime)) {
      manager::flightModeManager.changeMode(FlightModeManager::FlightMode::SHUTDOWN);
      // Serial.println("SHUTDOWN");
    }
    break;

  default:
    break;
  }

  indicateFlightMode(manager::flightModeManager.getActiveModeNumber());
}


bool canWakeUp() {
  return !device::flightPin.isOpen();
}


bool canIgnition() {
  return !device::flightPin.isOpen();
}


void indicateFlightMode(uint8_t mode) {
  digitalWrite(D5, (mode & (1 << 0)));
  digitalWrite(D6, (mode & (1 << 1)));
  digitalWrite(D7, (mode & (1 << 2)));
  digitalWrite(D8, (mode & (1 << 3)));
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