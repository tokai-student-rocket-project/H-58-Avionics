#include "Transmitter.hpp"


void Transmitter::begin(int32_t frequency, int32_t bandwidth) {
  LoRa.begin(frequency);
  LoRa.setSignalBandwidth(bandwidth);
}


void Transmitter::sendAirData(float altitude, float outsideTemperature, float orientation_x, float orientation_y, float orientation_z, float linear_acceleration_x, float linear_acceleration_y, float linear_acceleration_z) {
  const auto& packet = MsgPacketizer::encode(static_cast<uint8_t>(Index::AIR_DATA), altitude, outsideTemperature, orientation_x, orientation_y, orientation_z, linear_acceleration_x, linear_acceleration_y, linear_acceleration_z);
  sendDownlink(packet.data.data(), packet.data.size());
}


void Transmitter::sendPowerData(float supplyVoltage, float batteryVoltage, float poolVoltage) {
  const auto& packet = MsgPacketizer::encode(static_cast<uint8_t>(Index::POWER_DATA), supplyVoltage, batteryVoltage, poolVoltage);
  sendDownlink(packet.data.data(), packet.data.size());
}


void Transmitter::sendPositionData(float latitude, float longitude) {
  const auto& packet = MsgPacketizer::encode(static_cast<uint8_t>(Index::POSITION_DATA), latitude, longitude);
  sendDownlink(packet.data.data(), packet.data.size());
}


void Transmitter::sendSystemStatus(uint8_t flightMode, bool cameraState, bool sn3State, bool doLogging) {
  const auto& packet = MsgPacketizer::encode(static_cast<uint8_t>(Index::SYSTEM_STATUS), flightMode, cameraState, sn3State, doLogging);
  sendDownlink(packet.data.data(), packet.data.size());
}


void Transmitter::sendSensingStatus(float referencePressure, bool isSystemCalibrated, bool isGyroscopeCalibrated, bool isAccelerometerCalibrated, bool isMagnetometerCalibrated) {
  const auto& packet = MsgPacketizer::encode(static_cast<uint8_t>(Index::SENSING_STATUS), referencePressure, isSystemCalibrated, isGyroscopeCalibrated, isAccelerometerCalibrated, isMagnetometerCalibrated);
  sendDownlink(packet.data.data(), packet.data.size());
}


void Transmitter::sendEvent(uint8_t publisher, uint8_t eventCode, uint32_t timestamp) {
  const auto& packet = MsgPacketizer::encode(static_cast<uint8_t>(Index::EVENT), publisher, eventCode, timestamp);
  sendDownlink(packet.data.data(), packet.data.size());
}


void Transmitter::sendError(uint8_t publisher, uint8_t errorCode, uint8_t errorReason, uint32_t timestamp) {
  const auto& packet = MsgPacketizer::encode(static_cast<uint8_t>(Index::ERROR), publisher, errorCode, errorReason, timestamp);
  sendDownlink(packet.data.data(), packet.data.size());
}


void Transmitter::sendValveStatus(float currentPosition, float currentDesiredPosition, float currentVelocity, float mcuTemperature, float motorTemperature, float current, float inputVoltage) {
  const auto& packet = MsgPacketizer::encode(static_cast<uint8_t>(Index::VALVE_STATUS), currentPosition, currentDesiredPosition, currentVelocity, mcuTemperature, motorTemperature, current, inputVoltage);
  sendDownlink(packet.data.data(), packet.data.size());
}


void Transmitter::sendDownlink(const uint8_t* data, uint32_t size) {
  LoRa.beginPacket();
  LoRa.write(data, size);
  LoRa.endPacket();
}