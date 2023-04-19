#include <ACAN_STM32.h>
#include <TaskManager.h>
#include "CANBUS.hpp"
#include "FlightModeManager.hpp"
#include "TimerManager.hpp"
#include "PullupPin.hpp"


CANBUS canbus;

FlightModeManager _flightModeManager;
TimerManager _timerManager;

PullupPin _flightPin(D4);


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);

  canbus.initialize();

  Tasks.add(task100Hz)->startIntervalMsec(10);
}


void loop() {
  Tasks.update();

  if (canbus.isAvailable()) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    canbus.receive();
  }
}


void task100Hz() {
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