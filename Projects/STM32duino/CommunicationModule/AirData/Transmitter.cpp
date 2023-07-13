#include "Transmitter.hpp"


void Transmitter::begin(int32_t frequency, int32_t bandwidth) {
  LoRa.begin(frequency);
  LoRa.setSignalBandwidth(bandwidth);
}


void Transmitter::sendAirData(float altitude, float outsideTemperature, float orientation_x, float orientation_y, float orientation_z, float linear_acceleration_x, float linear_acceleration_y, float linear_acceleration_z) {
  const auto& packet = MsgPacketizer::encode(static_cast<uint8_t>(Index::AIR_DATA), altitude, outsideTemperature, orientation_x, orientation_y, orientation_z, linear_acceleration_x, linear_acceleration_y, linear_acceleration_z);
  sendDownlink(packet.data.data(), packet.data.size());
}


void Transmitter::sendDownlink(const uint8_t* data, uint32_t size) {
  LoRa.beginPacket();
  LoRa.write(data, size);
  LoRa.endPacket();
}