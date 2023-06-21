#pragma once


#include <Arduino.h>
#include <mcp2515_can.h>


class CANMCP {
public:
  enum class Label : uint32_t {
    TEMPERATURE,
    PRESSURE,
    ALTITUDE,
    ACCELERATION,
    GYROSCOPE,
    MAGNETOMETER,
    ORIENTATION,
    LINEAR_ACCELERATION,
    GRAVITY,
    STATUS,
    VOLTAGE_SUPPLY,
    VOLTAGE_BATTERY,
    VOLTAGE_POOL
  };

  enum class Axis : uint8_t {
    X,
    Y,
    Z
  };

  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  CANMCP(uint8_t cs);

  void begin();

  bool available();
  Label getLatestLabel();

  void receiveStatus(uint8_t* mode, bool* camera, bool* sn3, bool* sn4);
  void receiveScalar(float* value);
  void receiveVector(float* xValue, float* yValue, float* zValue);

private:
  mcp2515_can* _can;

  uint32_t _latestLabel;
  uint8_t _latestData[8];
};