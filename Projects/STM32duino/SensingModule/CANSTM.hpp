#pragma once


#include <Arduino.h>
#include <ACAN_STM32.h>


class CANSTM {
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

  void begin(uint32_t bitrate);

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
  CANMessage _latestMessage;
};