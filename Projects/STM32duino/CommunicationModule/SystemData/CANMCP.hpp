#pragma once


#include <Arduino.h>
#include <mcp2515_can.h>
#include "Var.hpp"


class CANMCP {
public:
  enum class Label : uint32_t {
    ORIENTATION,
    LINEAR_ACCELERATION,
    ALTITUDE,
    OUTSIDE_TEMPERATURE,
    VOLTAGE_SUPPLY,
    VOLTAGE_BATTERY,
    VOLTAGE_POOL,
    SYSTEM_STATUS,
    EVENT,
    ERROR,
    SET_REFERENCE_PRESSURE_COMMAND,
    TRAJECTORY_DATA,
    SENSING_STATUS,
    CURRENT_POSITION = 0x103,
    CURRENT_DESIRED_POSITION,
    CURRENT_VELOCITY,
    MCU_TEMPERATURE,
    MOTOR_TEMPERATURE,
    CURRENT,
    INPUT_VOLTAGE
  };

  enum class Publisher : uint8_t {
    SENSING_MODULE,
    FLIGHT_MODULE,
    MISSION_MODULE,
    AIR_DATA_COMMUNICATION_MODULE,
    SYSTEM_DATA_COMMUNICATION_MODULE
  };

  enum class EventCode : uint8_t {
    SETUP,
    RESET,
    FLIGHT_MODE_ON,
    IGNITION,
    BURNOUT,
    APOGEE,
    SEPARATE,
    LAND,
    FLIGHT_MODE_OFF,
    FORCE_SEPARATE,
    REFERENCE_PRESSURE_UPDATED
  };

  enum class ErrorCode : uint8_t {
    COMMAND_RECEIVE_FAILED,
    LOGGER_FAILURE
  };

  enum class ErrorReason : uint8_t {
    INVALID_KEY,
    INVALID_SD
  };


  CANMCP(uint8_t cs);

  void begin();

  bool available();
  Label getLatestLabel();

  void sendEvent(Publisher publisher, EventCode eventCode, uint32_t timestamp = 0);
  void sendError(Publisher publisher, ErrorCode errorCode, ErrorReason errorReason, uint32_t timestamp = 0);
  void sendSetReferencePressureCommand(float referencePressure);


  void receiveSystemStatus(Var::FlightMode* flightMode, Var::State* cameraState, Var::State* sn3State, bool* doLogging);
  void receiveSensingStatus(float* referencePressure, bool* isSystemCalibrated, bool* isGyroscopeCalibrated, bool* isAccelerometerCalibrated, bool* isMagnetometerCalibrated);
  void receiveScalar(float* value);
  void receiveVector(float* xValue, float* yValue, float* zValue);
  void receiveEvent(Publisher* publisher, EventCode* eventCode, uint32_t* timestamp);
  void receiveError(Publisher* publisher, ErrorCode* errorCode, ErrorReason* errorReason, uint32_t* timestamp);

  void receiveServo(float* value);

private:
  mcp2515_can* _can;

  uint32_t _latestLabel;
  uint8_t _latestData[8];
};