#include <ACAN_STM32.h>
#include <TaskManager.h>
#include "FlightModeManager.hpp"
#include "TimerManager.hpp"
#include "PullupPin.hpp"


FlightModeManager _flightModeManager;
TimerManager _timerManager;

PullupPin _flightPin(D4);


union Converter {
  float value;
  uint8_t data[4];
}converter;

float pressure;
float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;


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
    case 0x01:
      read(message, &pressure);
      break;
    case 0x6:
      readVector(message, &linear_acceleration_x, &linear_acceleration_y, &linear_acceleration_z);
      break;

    default:
      break;
    }
  }
}


void task100Hz() {
  Serial.print(linear_acceleration_x);
  Serial.print(",");
  Serial.print(linear_acceleration_y);
  Serial.print(",");
  Serial.println(linear_acceleration_z);

  _flightPin.update();

  switch (_flightModeManager.getActiveMode()) {
  case (FlightModeManager::FlightMode::SLEEP):
    if (canWakeUp()) {
      _flightModeManager.changeMode(FlightModeManager::FlightMode::STANDBY);
      // Serial.println("WAKE_UP");
    }
    break;

  case (FlightModeManager::FlightMode::STANDBY):
    if (canIgnition()) {
      _flightModeManager.changeMode(FlightModeManager::FlightMode::THRUST);
      // Serial.println("IGNITION");

      _timerManager.setReferenceTime();
    }
    break;

  case (FlightModeManager::FlightMode::THRUST):
    if (_timerManager.isElapsedTime(_timerManager.ThrustTime)) {
      _flightModeManager.changeMode(FlightModeManager::FlightMode::CLIMB);
      // Serial.println("BURNOUT");
    }
    break;

  case (FlightModeManager::FlightMode::CLIMB):
    if (_timerManager.isElapsedTime(_timerManager.ApogeeTime)) {
      _flightModeManager.changeMode(FlightModeManager::FlightMode::DESCENT);
      // Serial.println("APOGEE");
    }
    break;

  case (FlightModeManager::FlightMode::DESCENT):
    if (_timerManager.isElapsedTime(_timerManager.FirstSeparationTime)) {
      _flightModeManager.changeMode(FlightModeManager::FlightMode::DECEL);
      // Serial.println("1ST_SEPARATION");
    }
    break;

  case (FlightModeManager::FlightMode::DECEL):
    if (_timerManager.isElapsedTime(_timerManager.SecondSeparationTime)) {
      _flightModeManager.changeMode(FlightModeManager::FlightMode::PARACHUTE);
      // Serial.println("2ND_SEPARATION");
    }
    break;

  case (FlightModeManager::FlightMode::PARACHUTE):
    if (_timerManager.isElapsedTime(_timerManager.LandTime)) {
      _flightModeManager.changeMode(FlightModeManager::FlightMode::LAND);
      // Serial.println("LAND");
    }
    break;

  case (FlightModeManager::FlightMode::LAND):
    if (_timerManager.isElapsedTime(_timerManager.ShutdownTime)) {
      _flightModeManager.changeMode(FlightModeManager::FlightMode::SHUTDOWN);
      // Serial.println("SHUTDOWN");
    }
    break;

  default:
    break;
  }

  indicateFlightMode(_flightModeManager.getActiveModeNumber());
}


bool canWakeUp() {
  return !_flightPin.isOpen();
}


bool canIgnition() {
  return !_flightPin.isOpen();
}


void indicateFlightMode(uint8_t mode) {
  digitalWrite(D5, (mode & (1 << 0)));
  digitalWrite(D6, (mode & (1 << 1)));
  digitalWrite(D7, (mode & (1 << 2)));
  digitalWrite(D8, (mode & (1 << 3)));
}


void read(CANMessage message, float* value) {
  converter.data[0] = message.data[0];
  converter.data[1] = message.data[1];
  converter.data[2] = message.data[2];
  converter.data[3] = message.data[3];
  *value = converter.value;
}


void readVector(CANMessage message, float* x, float* y, float* z) {
  converter.data[0] = message.data[1];
  converter.data[1] = message.data[2];
  converter.data[2] = message.data[3];
  converter.data[3] = message.data[4];

  switch (message.data[0]) {
  case 0:
    *x = converter.value;
    break;
  case 1:
    *y = converter.value;
    break;
  case 2:
    *z = converter.value;
    break;

  default:
    break;
  }
}