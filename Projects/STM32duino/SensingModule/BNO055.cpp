#include "BNO055.hpp"


void BNO055::initialize() {
  // OPR_MODEをCONFIGMODEに変更
  Wire.beginTransmission(BNO055::SLAVE_ADDRESS);
  Wire.write(BNO055::REGISTER_OPR_MODE);
  Wire.write(BNO055::OPR_MODE_CONFIGMODE);
  Wire.endTransmission();
  delay(50);

  // PWR_MODEをNormal Modeに変更
  Wire.beginTransmission(BNO055::SLAVE_ADDRESS);
  Wire.write(BNO055::REGISTER_PWR_MODE);
  Wire.write(BNO055::PWR_MODE_NORMALMODE);
  Wire.endTransmission();
  delay(10);

  // PAGEを0に変更
  Wire.beginTransmission(BNO055::SLAVE_ADDRESS);
  Wire.write(BNO055::REGISTER_PAGE);
  Wire.write(0);
  Wire.endTransmission();

  //SYS_TRIGGERのCLK_SELをトリガー
  Wire.beginTransmission(BNO055::SLAVE_ADDRESS);
  Wire.write(BNO055::REGISTER_SYS_TRIGGER);
  Wire.write(BNO055::CLK_SEL);
  Wire.endTransmission();
  delay(10);

  // OPR_MODEをNDOFに変更
  Wire.beginTransmission(BNO055::SLAVE_ADDRESS);
  Wire.write(BNO055::REGISTER_OPR_MODE);
  Wire.write(BNO055::OPR_MODE_NDOF);
  Wire.endTransmission();
  delay(50);
}


void BNO055::getAcceleration(double* x, double* y, double* z) {
  Wire.beginTransmission(BNO055::SLAVE_ADDRESS);
  Wire.write(BNO055::RESISTER_ACC_DATA);
  Wire.endTransmission();
  Wire.requestFrom(BNO055::SLAVE_ADDRESS, 6);

  int16_t xRaw, yRaw, zRaw;

  xRaw = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);
  yRaw = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);
  zRaw = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);

  *x = ((double)xRaw) / 100.0;
  *y = ((double)yRaw) / 100.0;
  *z = ((double)zRaw) / 100.0;
}