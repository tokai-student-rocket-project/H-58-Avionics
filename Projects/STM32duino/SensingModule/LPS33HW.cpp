#include "LPS33HW.hpp"


LPS33HW::LPS33HW(TwoWire* wire, uint8_t address) {
  _wire = wire;
  _address = address;
}


void LPS33HW::initialize() {
  writeByte(REGISTER::CTRL_REG1, 0b0101000);
}


void LPS33HW::getPressure(raw_t* pressure) {
  requestBytes(REGISTER::PRESS_OUT_XL, 3);
  *pressure = { Wire.read(), Wire.read(), Wire.read() };
}


void LPS33HW::writeByte(register_t reg, uint8_t content) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.write(content);
  Wire.endTransmission();
}


void LPS33HW::requestBytes(register_t reg, uint8_t length) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(_address, length);
}