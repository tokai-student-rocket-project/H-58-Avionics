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
    EVENT
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
    FLIGHT_MODE_OFF
  };

  CANMCP(uint8_t cs);

  void begin();

  bool available();
  Label getLatestLabel();

  void sendEvent(Publisher publisher, EventCode eventCode, uint32_t time = 0, uint16_t payload = 0);


  void receiveStatus(uint8_t* mode, bool* camera, bool* sn3);
  void receiveScalar(float* value);
  void receiveVector(float* xValue, float* yValue, float* zValue);
  void receiveEvent(Publisher* publisher, EventCode* eventCode, uint32_t* time, uint16_t* payload);

private:
  mcp2515_can* _can;

  uint32_t _latestLabel;
  uint8_t _latestData[8];
};