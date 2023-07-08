#pragma once


#include <Arduino.h>
#include <MsgPacketizer.h>
#include "FRAM.hpp"


class Recorder {
public:
  Recorder(uint32_t csFram0, uint32_t csFram1);

  void reset();
  void dump();
  void clear();

  void record(
    uint32_t millis,
    float temperature, float pressure, float altitude, float climbIndex, bool isFalling,
    float acceleration_x, float acceleration_y, float acceleration_z,
    float gyroscope_x, float gyroscope_y, float gyroscope_z,
    float magnetometer_x, float magnetometer_y, float magnetometer_z,
    float orientation_x, float orientation_y, float orientation_z,
    float linear_acceleration_x, float linear_acceleration_y, float linear_acceleration_z,
    float gravity_x, float gravity_y, float gravity_z
  );

private:
  uint32_t _offset = 0;

  FRAM* _fram0;
  FRAM* _fram1;
};