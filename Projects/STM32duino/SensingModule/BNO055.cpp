#include "BNO055.hpp"


/// @brief BME280の抽象化クラス
void BNO055::begin() {
  _bno.begin();
  _bno.setExtCrystalUse(true);
}


/// @brief 現在の加速度ベクトルを返す
/// @param x x軸加速度のポインタ [mps2]
/// @param y y軸加速度のポインタ [mps2]
/// @param z y軸加速度のポインタ [mps2]
void BNO055::getAcceleration(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  // 座標軸を合わせるためにxyzを入れ替えているので注意
  *x = event.acceleration.z;
  *y = event.acceleration.x;
  *z = event.acceleration.y;
}


/// @brief 現在の地磁気を返す
/// @param x x軸地磁気のポインタ [nT]
/// @param y y軸地磁気のポインタ [nT]
/// @param z y軸地磁気のポインタ [nT]
void BNO055::getMagnetometer(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  // 座標軸を合わせるためにxyzを入れ替えているので注意
  *x = event.magnetic.z;
  *y = event.magnetic.x;
  *z = event.magnetic.y;
}


/// @brief 現在の角加速度を返す
/// @param x x軸周り角加速度のポインタ [dps]
/// @param y y軸周り角加速度のポインタ [dps]
/// @param z y軸周り角加速度のポインタ [dps]
void BNO055::getGyroscope(float* x, float* y, float* z) {
  // 座標軸を合わせるためにxyzを入れ替えているので注意
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_GYROSCOPE);
  *x = event.gyro.z;
  *y = event.gyro.x;
  *z = event.gyro.y;
}


/// @brief 現在の重力加速度を返す
/// @param x x軸重力加速度のポインタ [mps2]
/// @param y y軸重力加速度のポインタ [mps2]
/// @param z y軸重力加速度のポインタ [mps2]
void BNO055::getGravityVector(float* x, float* y, float* z) {
  // 座標軸を合わせるためにxyzを入れ替えているので注意
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_GRAVITY);
  *x = event.acceleration.z;
  *y = event.acceleration.x;
  *z = event.acceleration.y;
}


/// @brief 現在の線形加速度を返す
/// @param x x軸線形加速度のポインタ [mps2]
/// @param y y軸線形加速度のポインタ [mps2]
/// @param z y軸線形加速度のポインタ [mps2]
void BNO055::getLinearAcceleration(float* x, float* y, float* z) {
  // 座標軸を合わせるためにxyzを入れ替えているので注意
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_LINEARACCEL);
  *x = event.acceleration.z;
  *y = event.acceleration.x;
  *z = event.acceleration.y;
}


/// @brief 現在の姿勢角を返す
/// @param x x軸(Roll)姿勢角のポインタ [deg]
/// @param y y軸(Pitch)姿勢角のポインタ [deg]
/// @param z y軸(Yaw)姿勢角のポインタ [deg]
void BNO055::getOrientation(float* x, float* y, float* z) {
  // 座標軸を合わせるためにxyzを入れ替えているので注意
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_EULER);
  *x = -event.orientation.z;
  *y = -event.orientation.x;
  *z = -event.orientation.y;
}