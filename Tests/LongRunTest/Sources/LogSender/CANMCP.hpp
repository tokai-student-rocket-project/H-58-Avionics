#pragma once


#include <Arduino.h>
#include <mcp2515_can.h>


class CANMCP {
public:
  enum class Label : uint32_t {
    ORIENTATION,
    LINEAR_ACCELERATION,
    ALTITUDE,
    TEMPERATURE,
    VOLTAGE_SUPPLY,
    VOLTAGE_BATTERY,
    VOLTAGE_POOL,
    SYSTEM_STATUS,
    EVENT,
    ERROR,
    SET_REFERENCE_PRESSURE_COMMAND
  };

  enum class Axis : uint8_t {
    X,
    Y,
    Z
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
    COMMAND_RECEIVE_FAILED
  };

  enum class ErrorReason : uint8_t {
    INVALID_KEY
  };


  CANMCP(uint8_t cs);

  void begin();

  bool available();
  Label getLatestLabel();

  void sendEvent(Publisher publisher, EventCode eventCode, uint32_t timestamp = 0);
  void sendError(Publisher publisher, ErrorCode errorCode, ErrorReason errorReason, uint32_t timestamp = 0);
  void sendSetReferencePressureCommand(float referencePressure);


  void receiveStatus(uint8_t* mode, bool* camera, bool* sn3);
  void receiveScalar(float* value);
  void receiveVector(float* xValue, float* yValue, float* zValue);
  void receiveEvent(Publisher* publisher, EventCode* eventCode, uint32_t* timestamp);
  void receiveError(Publisher* publisher, ErrorCode* errorCode, ErrorReason* errorReason, uint32_t* timestamp);

private:
  mcp2515_can* _can;

  uint32_t _latestLabel;
  uint8_t _latestData[8];
};