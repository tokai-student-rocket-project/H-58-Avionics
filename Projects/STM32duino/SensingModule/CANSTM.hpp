#pragma once


#include <Arduino.h>
#include <ACAN_STM32.h>


class CANSTM {
public:
  enum class Label : uint32_t {
    ORIENTATION,
    LINEAR_ACCELERATION,
    ALTITUDE,
    TEMPERATURE,
    VOLTAGE_SUPPLY,
    VOLTAGE_BATTERY,
    VOLTAGE_POOL,
    SYSTEM_STATUS
  };

  enum class Axis : uint8_t { X, Y, Z };

  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  void begin();

  bool available();
  Label getLatestMessageLabel();

  void sendStatus(Label id, uint8_t mode, bool camera, bool sn3, bool sn4);
  void sendScalar(Label label, float value);
  void sendVector(Label label, Axis axis, float value);
  void sendVector3D(Label label, float xValue, float yValue, float zValue);

  void receiveStatus(uint8_t* mode, bool* camera, bool* sn3, bool* sn4);
  void receiveScalar(float* value);
  void receiveVector(float* xValue, float* yValue, float* zValue);

private:
  uint32_t _latestLabel;
  uint8_t _latestData[8];
};