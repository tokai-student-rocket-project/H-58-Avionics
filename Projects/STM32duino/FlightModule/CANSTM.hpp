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
    SYSTEM_STATUS,
    EVENT,
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


  void begin();

  bool available();
  Label getLatestMessageLabel();

  void sendSystemStatus(uint8_t mode, bool camera, bool sn3);
  void sendEvent(Publisher publisher, EventCode eventCode, uint32_t timestamp = 0);

  void sendScalar(Label label, float value);
  void sendVector(Label label, Axis axis, float value);
  void sendVector3D(Label label, float xValue, float yValue, float zValue);

  void receiveStatus(uint8_t* mode, bool* camera, bool* sn3);
  void receiveScalar(float* value);
  void receiveVector(float* xValue, float* yValue, float* zValue);
  void receiveSetReferencePressureCommand(float* referencePressure);

private:
  uint32_t _latestLabel;
  uint8_t _latestData[8];
};