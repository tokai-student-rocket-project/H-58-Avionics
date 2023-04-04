#ifndef BNO_055_HPP_
#define BNO_055_HPP_


#include <Wire.h>


class BNO055 {
public:
  typedef enum  REGISTER {
    PAGE_ID = 0x07,
    ACC_DATA = 0x08,
    OPR_MODE = 0x3D,
    PWR_MODE = 0x3E,
    SYS_TRIGGER = 0x3F,
  } register_t;

  typedef enum  OPR_MODE {
    CONFIGMODE = 0b00000000,
    NDOF = 0b00001100,
  } opr_mode_t;

  typedef enum PWR_MODE {
    NORMALMODE = 0b00000000,
  } pwr_mode_t;

  typedef enum SYS_TRIGGER {
    CLK_SEL = 0b10000000,
  } sys_trigger_t;

  BNO055(TwoWire* wire, uint8_t address);
  void initialize();

  void setPage(uint8_t pageId);
  void setOperatingMode(opr_mode_t operatingMode);
  void setPowerMode(pwr_mode_t powerMode);
  void setClockSelect(bool isExternalCrystal);

  void getAcceleration(double* x, double* y, double* z);

private:
  TwoWire* _wire;
  uint8_t _address;

  void writeByte(register_t reg, uint8_t content);
  void requestBytes(register_t reg, uint8_t length);
};


#endif