#pragma once


#include <Arduino.h>
#include <LoRa.h>
#include <MsgPacketizer.h>


/// @brief LoRa管理用のクラス
class Transmitter {
public:
public:
  enum class Index : uint8_t {
    AIR_DATA
  };

  void begin(int32_t frequency, int32_t bandwidth);

  void sendAirData(float altitude, float outsideTemperature, float orientation_x, float orientation_y, float orientation_z, float linear_acceleration_x, float linear_acceleration_y, float linear_acceleration_z);

private:
  void sendDownlink(const uint8_t* data, uint32_t size);
};