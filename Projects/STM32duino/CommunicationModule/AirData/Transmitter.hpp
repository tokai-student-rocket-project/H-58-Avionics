#pragma once


#include <Arduino.h>
#include <LoRa.h>
#include <MsgPacketizer.h>


/// @brief LoRa管理用のクラス
class Transmitter {
public:
public:
  enum class Index : uint8_t {
    AIR_DATA,
    POWER_DATA,
    POSITION_DATA,
    SYSTEM_STATUS,
    SENSING_STATUS,
    EVENT,
    ERROR,
    VALVE_STATUS
  };

  void begin(int32_t frequency, int32_t bandwidth);

  void sendAirData(float altitude, float outsideTemperature, float orientation_x, float orientation_y, float orientation_z, float linear_acceleration_x, float linear_acceleration_y, float linear_acceleration_z);
  void sendPowerData(float supplyVoltage, float batteryVoltage, float poolVoltage);
  void sendPositionData(float latitude, float longitude);
  void sendSystemStatus(uint8_t flightMode, bool cameraState, bool sn3State, bool doLogging);
  void sendSensingStatus(float referencePressure, bool isSystemCalibrated, bool isGyroscopeCalibrated, bool isAccelerometerCalibrated, bool isMagnetometerCalibrated);
  void sendEvent(uint8_t publisher, uint8_t eventCode, uint32_t timestamp);
  void sendError(uint8_t publisher, uint8_t errorCode, uint8_t errorReason, uint32_t timestamp);
  void sendValveStatus(float currentPosition, float currentDesiredPosition, float currentVelocity, float mcuTemperature, float motorTemperature, float current, float inputVoltage);

private:
  void sendDownlink(const uint8_t* data, uint32_t size);
};