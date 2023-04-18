#pragma once


#include <Wire.h>
#include "DataType.hpp"


class LPS33HW {
public:
  typedef enum  REGISTER {
    CTRL_REG1 = 0x10,
    PRESS_OUT_XL = 0x28
  } register_t;

  LPS33HW(TwoWire* wire, uint8_t address);
  void initialize();

  void getPressure(raw_t* pressure);

private:
  TwoWire* _wire;
  uint8_t _address;

  void writeByte(register_t reg, uint8_t content);
  void requestBytes(register_t reg, uint8_t length);
};