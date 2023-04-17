#include "BNO055.hpp"


BNO055::BNO055(TwoWire* wire, uint8_t address) {
  _wire = wire;
  _address = address;
}


void BNO055::initialize() {
  setClockSelect(true);
  setPage(1);
  setAccelerometerConfig(ACC_CONFIG_G_RANGE_16G, ACC_CONFIG_BANDWIDTH_1000Hz, ACC_CONFIG_OPERATION_MODE_NORMAL);
  setGyroscopeConfig(GYR_CONFIG_RANGE_2000DPS, GYR_CONFIG_BANDWIDTH_523Hz, GYR_CONFIG_OPERATION_MODE_NORMAL);
  setMagnetometerConfig(MAG_CONFIG_DATA_OUTPUT_RATE_30HZ, MAG_CONFIG_OPERATION_MODE_HIGH_ACCURACY, MAG_CONFIG_POWER_MODE_NORMAL);
  setPage(0);
  setOperatingMode(OPR_MODE_NDOF);
}


void BNO055::setPage(uint8_t pageId) {
  writeByte(REGISTER::PAGE_ID, pageId);
}


void BNO055::setOperatingMode(opr_mode_t operatingMode) {
  writeByte(REGISTER::OPR_MODE, operatingMode);
  delay(50);
}


void BNO055::setPowerMode(pwr_mode_t powerMode) {
  writeByte(REGISTER::PWR_MODE, powerMode);
}


void BNO055::setClockSelect(bool isExternalCrystal) {
  writeByte(REGISTER::SYS_TRIGGER, isExternalCrystal ? SYS_TRIGGER_CLK_SEL : 0x00);
}


void BNO055::setAccelerometerConfig(acc_config_g_range_t gRange, acc_config_bandwidth_t bandwidth, acc_config_operation_mode_t operationMode) {
  writeByte(REGISTER::ACC_CONFIG, gRange | bandwidth | operationMode);
}


void BNO055::setGyroscopeConfig(gyr_config_range_t range, gyr_config_bandwidth_t bandwidth, gyr_config_operation_mode_t operationMode) {
  writeByte(REGISTER::ACC_CONFIG, range | bandwidth | operationMode);
}


void BNO055::setMagnetometerConfig(mag_config_data_output_rate_t dataOutputRate, mag_config_operation_mode_t operationMode, mag_config_power_mode_t powerMode) {
  writeByte(REGISTER::ACC_CONFIG, dataOutputRate | operationMode | powerMode);
}


void BNO055::getAcceleration(raw_t* x, raw_t* y, raw_t* z) {
  requestBytes(REGISTER::ACC_DATA, 6);
  *x = { Wire.read(), Wire.read() };
  *y = { Wire.read(), Wire.read() };
  *z = { Wire.read(), Wire.read() };
}


void BNO055::getMagnetometer(raw_t* x, raw_t* y, raw_t* z) {
  requestBytes(REGISTER::MAG_DATA, 6);
  *x = { Wire.read(), Wire.read() };
  *y = { Wire.read(), Wire.read() };
  *z = { Wire.read(), Wire.read() };
}


void BNO055::getGyroscope(raw_t* x, raw_t* y, raw_t* z) {
  requestBytes(REGISTER::GYR_DATA, 6);
  *x = { Wire.read(), Wire.read() };
  *y = { Wire.read(), Wire.read() };
  *z = { Wire.read(), Wire.read() };
}


void BNO055::getQuaternion(raw_t* w, raw_t* x, raw_t* y, raw_t* z) {
  requestBytes(REGISTER::QUA_DATA, 8);
  *w = { Wire.read(), Wire.read() };
  *x = { Wire.read(), Wire.read() };
  *y = { Wire.read(), Wire.read() };
  *z = { Wire.read(), Wire.read() };
}


void BNO055::getGravityVector(raw_t* x, raw_t* y, raw_t* z) {
  requestBytes(REGISTER::GRV_DATA, 6);
  *x = { Wire.read(), Wire.read() };
  *y = { Wire.read(), Wire.read() };
  *z = { Wire.read(), Wire.read() };
}


void BNO055::getLinearAcceleration(raw_t* x, raw_t* y, raw_t* z) {
  requestBytes(REGISTER::LIA_DATA, 6);
  *x = { Wire.read(), Wire.read() };
  *y = { Wire.read(), Wire.read() };
  *z = { Wire.read(), Wire.read() };
}


void BNO055::getEuler(raw_t* heading, raw_t* roll, raw_t* pitch) {
  requestBytes(REGISTER::EUL_DATA, 6);
  *heading = { Wire.read(), Wire.read() };
  *roll = { Wire.read(), Wire.read() };
  *pitch = { Wire.read(), Wire.read() };
}


void BNO055::writeByte(register_t reg, uint8_t content) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.write(content);
  Wire.endTransmission();
}


void BNO055::requestBytes(register_t reg, uint8_t length) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(_address, length);
}