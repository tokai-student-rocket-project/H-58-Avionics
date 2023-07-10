#include "BNO055.hpp"


/// @brief センサを初期化して起動する
void BNO055::begin() {
  delay(850);

  // OPR_MODE <- Config mode
  write(0x3D, 0x00);
  delay(30);

  // PWR_MODE <- Normal mode
  write(0x3E, 0x00);
  delay(10);

  // PAGE <- 0
  write(0x07, 0);

  // SYS_TRIGGER <- Use external oscillator
  write(0x3F, 0b10000000);
  delay(10);

  // OPR_MODE <- NDOF mode
  write(0x3D, 0x0C);
  delay(20);
}


/// @brief 現在の加速度ベクトルを返す
/// @param x x軸加速度のポインタ [mps2]
/// @param y y軸加速度のポインタ [mps2]
/// @param z y軸加速度のポインタ [mps2]
void BNO055::getAcceleration(float* x, float* y, float* z) {
  readVector3D(0x08, 100.0, x, y, z);
}


/// @brief 現在の地磁気を返す
/// @param x x軸地磁気のポインタ [nT]
/// @param y y軸地磁気のポインタ [nT]
/// @param z y軸地磁気のポインタ [nT]
void BNO055::getMagnetometer(float* x, float* y, float* z) {
  readVector3D(0x0E, 16.0, x, y, z);
}


/// @brief 現在の角加速度を返す
/// @param x x軸周り角加速度のポインタ [dps]
/// @param y y軸周り角加速度のポインタ [dps]
/// @param z y軸周り角加速度のポインタ [dps]
void BNO055::getGyroscope(float* x, float* y, float* z) {
  readVector3D(0x14, 16.0, x, y, z);
}


/// @brief 現在の重力加速度を返す
/// @param x x軸重力加速度のポインタ [mps2]
/// @param y y軸重力加速度のポインタ [mps2]
/// @param z y軸重力加速度のポインタ [mps2]
void BNO055::getGravityVector(float* x, float* y, float* z) {
  readVector3D(0x2E, 100.0, x, y, z);
}


/// @brief 現在の線形加速度を返す
/// @param x x軸線形加速度のポインタ [mps2]
/// @param y y軸線形加速度のポインタ [mps2]
/// @param z y軸線形加速度のポインタ [mps2]
void BNO055::getLinearAcceleration(float* x, float* y, float* z) {
  readVector3D(0x28, 100.0, x, y, z);
}


/// @brief 現在の姿勢角を返す
/// @param x x軸(Roll)姿勢角のポインタ [deg]
/// @param y y軸(Pitch)姿勢角のポインタ [deg]
/// @param z y軸(Yaw)姿勢角のポインタ [deg]
void BNO055::getOrientation(float* x, float* y, float* z) {
  readVector3D(0x1A, 16.0, x, y, z);
}


void BNO055::write(uint8_t address, uint8_t data) {
  Wire.beginTransmission(0x28);
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}


void BNO055::readVector3D(uint8_t address, float lsb, float* x, float* y, float* z) {
  Wire.beginTransmission(0x28);
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(0x28, 6);

  int16_t xRaw = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);
  int16_t yRaw = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);
  int16_t zRaw = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);

  // 座標軸を合わせるためにxyzを入れ替えているので注意
  *x = ((float)zRaw) / lsb;
  *y = ((float)xRaw) / lsb;
  *z = ((float)yRaw) / lsb;
}