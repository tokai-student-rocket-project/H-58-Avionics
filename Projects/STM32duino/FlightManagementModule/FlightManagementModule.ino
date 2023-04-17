#include <ACAN_STM32.h>
#include <TaskManager.h>
#include "FlightModeManager.hpp"
#include "TimerManager.hpp"
#include "PullupPin.hpp"


namespace manager {
  FlightModeManager _flightModeManager;
  TimerManager _timerManager;
}

namespace detection {
  PullupPin _flightPin(D4);
}


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  Tasks.add(task100Hz)->startIntervalMsec(10);
}


void loop() {
  Tasks.update();
}


void task100Hz() {
  detection::_flightPin.update();

  switch (manager::_flightModeManager.getActiveMode()) {
  case (FlightModeManager::FlightMode::SLEEP):
    if (canWakeUp()) {
      manager::_flightModeManager.changeMode(FlightModeManager::FlightMode::STANDBY);
      Serial.println("WAKE_UP");
    }
    break;

  case (FlightModeManager::FlightMode::STANDBY):
    if (canIgnition()) {
      manager::_flightModeManager.changeMode(FlightModeManager::FlightMode::THRUST);
      Serial.println("IGNITION");

      manager::_timerManager.setReferenceTime();
    }
    break;

  case (FlightModeManager::FlightMode::THRUST):
    if (manager::_timerManager.isElapsedTime(manager::_timerManager.ThrustTime)) {
      manager::_flightModeManager.changeMode(FlightModeManager::FlightMode::CLIMB);
      Serial.println("BURNOUT");
    }
    break;

  case (FlightModeManager::FlightMode::CLIMB):
    if (manager::_timerManager.isElapsedTime(manager::_timerManager.ApogeeTime)) {
      manager::_flightModeManager.changeMode(FlightModeManager::FlightMode::DESCENT);
      Serial.println("APOGEE");
    }
    break;

  case (FlightModeManager::FlightMode::DESCENT):
    if (manager::_timerManager.isElapsedTime(manager::_timerManager.FirstSeparationTime)) {
      manager::_flightModeManager.changeMode(FlightModeManager::FlightMode::DECEL);
      Serial.println("1ST_SEPARATION");
    }
    break;

  case (FlightModeManager::FlightMode::DECEL):
    if (manager::_timerManager.isElapsedTime(manager::_timerManager.SecondSeparationTime)) {
      manager::_flightModeManager.changeMode(FlightModeManager::FlightMode::PARACHUTE);
      Serial.println("2ND_SEPARATION");
    }
    break;

  case (FlightModeManager::FlightMode::PARACHUTE):
    if (manager::_timerManager.isElapsedTime(manager::_timerManager.LandTime)) {
      manager::_flightModeManager.changeMode(FlightModeManager::FlightMode::LAND);
      Serial.println("LAND");
    }
    break;

  case (FlightModeManager::FlightMode::LAND):
    if (manager::_timerManager.isElapsedTime(manager::_timerManager.ShutdownTime)) {
      manager::_flightModeManager.changeMode(FlightModeManager::FlightMode::SHUTDOWN);
      Serial.println("SHUTDOWN");
    }
    break;

  default:
    break;
  }
}


bool canWakeUp() {
  return detection::_flightPin.isOpen();
}


bool canIgnition() {
  return !detection::_flightPin.isOpen();
}