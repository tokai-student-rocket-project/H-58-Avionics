#ifndef BNO_055_HPP_
#define BNO_055_HPP_


#include <Wire.h>


class BNO055 {
public:
  typedef enum  REGISTER {
    PAGE_ID = 0x07,

    // PAGE 0
    ACC_DATA = 0x08,
    MAG_DATA = 0x0E,
    GYR_DATA = 0x14,
    EUL_DATA = 0x1A,
    QUA_DATA = 0x20,
    LIA_DATA = 0x28,
    GRV_DATA = 0x2E,
    OPR_MODE = 0x3D,
    PWR_MODE = 0x3E,
    SYS_TRIGGER = 0x3F,

    // PAGE 1
    ACC_CONFIG = 0x08,
    MAG_CONFIG = 0x09,
    GYR_CONFIG_0 = 0x0A,
    GYR_CONFIG_1 = 0x0B,
  } register_t;

  typedef enum {
    OPR_MODE_CONFIGMODE = 0b00000000,
    OPR_MODE_NDOF = 0b00001100,
  } opr_mode_t;

  typedef enum {
    PWR_MODE_NORMALMODE = 0b00000000,
  } pwr_mode_t;

  typedef enum {
    SYS_TRIGGER_CLK_SEL = 0b10000000,
  } sys_trigger_t;

  typedef enum {
    ACC_CONFIG_G_RANGE_16G = 0b00000011,
  } acc_config_g_range_t;

  typedef enum {
    ACC_CONFIG_BANDWIDTH_1000Hz = 0b00011100,
  } acc_config_bandwidth_t;

  typedef enum {
    ACC_CONFIG_OPERATION_MODE_NORMAL = 0b00000000,
  } acc_config_operation_mode_t;

  typedef enum {
    GYR_CONFIG_RANGE_2000DPS = 0b00000000,
  } gyr_config_range_t;

  typedef enum {
    GYR_CONFIG_BANDWIDTH_523Hz = 0b00000000,
  } gyr_config_bandwidth_t;

  typedef enum {
    GYR_CONFIG_OPERATION_MODE_NORMAL = 0b00000000,
  } gyr_config_operation_mode_t;

  typedef enum {
    MAG_CONFIG_DATA_OUTPUT_RATE_30HZ = 0b00000111,
  } mag_config_data_output_rate_t;

  typedef enum {
    MAG_CONFIG_OPERATION_MODE_HIGH_ACCURACY = 0b00011000,
  } mag_config_operation_mode_t;

  typedef enum {
    MAG_CONFIG_POWER_MODE_NORMAL = 0b00000000,
  } mag_config_power_mode_t;

  BNO055(TwoWire* wire, uint8_t address);
  void initialize();

  void setPage(uint8_t pageId);
  void setOperatingMode(opr_mode_t operatingMode);
  void setPowerMode(pwr_mode_t powerMode);
  void setClockSelect(bool isExternalCrystal);

  void setAccelerometerConfig(acc_config_g_range_t gRange, acc_config_bandwidth_t bandwidth, acc_config_operation_mode_t operationMode);
  void setGyroscopeConfig(gyr_config_range_t range, gyr_config_bandwidth_t bandwidth, gyr_config_operation_mode_t operationMode);
  void setMagnetometerConfig(mag_config_data_output_rate_t dataOutputRate, mag_config_operation_mode_t operationMode, mag_config_power_mode_t powerMode);

  void getAcceleration(double* x, double* y, double* z);
  void getMagnetometer(double* x, double* y, double* z);
  void getGyroscope(double* x, double* y, double* z);
  void getEuler(
    uint8_t* headingLSB, uint8_t* headingMSB,
    uint8_t* rollLSB, uint8_t* rollMSB,
    uint8_t* pitchLSB, uint8_t* pitchMSB);
  void getQuaternion(double* w, double* x, double* y, double* z);
  void getLinearAcceleration(double* x, double* y, double* z);
  void getGravityVector(double* x, double* y, double* z);

private:
  TwoWire* _wire;
  uint8_t _address;

  void writeByte(register_t reg, uint8_t content);
  void readVector3(register_t reg, double* x, double* y, double* z, double lsb);
  void readVector4(register_t reg, double* w, double* x, double* y, double* z, double lsb);
};


#endif