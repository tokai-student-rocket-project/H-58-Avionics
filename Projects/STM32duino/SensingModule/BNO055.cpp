#include "BNO055.hpp"


BNO055::BNO055(TwoWire* wire, uint8_t address) {
  _wire = wire;
  _address = address;
}


void BNO055::initialize() {
  setOperatingMode(OPR_MODE::CONFIGMODE);
  setPowerMode(PWR_MODE::NORMALMODE);
  setPage(0);
  setClockSelect(true);
  setOperatingMode(OPR_MODE::NDOF);
}


void BNO055::setPage(uint8_t pageId) {
  writeByte(REGISTER::PAGE_ID, pageId);
  delay(10);
}


void BNO055::setOperatingMode(opr_mode_t operatingMode) {
  writeByte(REGISTER::OPR_MODE, operatingMode);
  delay(50);
}


void BNO055::setPowerMode(pwr_mode_t powerMode) {
  writeByte(REGISTER::PWR_MODE, powerMode);
  delay(10);
}


void BNO055::setClockSelect(bool isExternalCrystal) {
  writeByte(REGISTER::SYS_TRIGGER, isExternalCrystal ? SYS_TRIGGER::CLK_SEL : 0x00);
  delay(10);
}


void BNO055::getAcceleration(double* x, double* y, double* z) {
  requestBytes(REGISTER::ACC_DATA, 6);

  int16_t xRaw, yRaw, zRaw;
  xRaw = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);
  yRaw = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);
  zRaw = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);

  *x = ((double)xRaw) / 100.0;
  *y = ((double)yRaw) / 100.0;
  *z = ((double)zRaw) / 100.0;
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