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

  enum class Axis : uint8_t { X, Y, Z };

  union Converter {
    float value;
    uint32_t value_uint32;
    uint8_t data[4];
  }converter;

  CANMCP(uint8_t cs);

  void begin();

  bool available();
  Label getLatestLabel();

  void receiveStatus(uint8_t* mode, bool* camera, bool* sn3);
  void receiveScalar(float* value);
  void receiveVector(float* xValue, float* yValue, float* zValue);
  void receiveEvent(uint32_t* time, char* event);

private:
  mcp2515_can* _can;

  uint32_t _latestLabel;
  uint8_t _latestData[8];
};