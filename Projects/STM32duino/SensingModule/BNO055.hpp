#pragma once


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include "Vector3D.hpp"


/// @brief BME055の抽象化クラス
class BNO055 {
public:
  /// @brief BME280の抽象化クラス
  void begin();


  /// @brief 現在の加速度ベクトルを返す
/// @param acceleration_mps2 加速度ベクトルのポインタ [mps2]
  void getAcceleration(Vector3D* acceleration_mps2);

  /// @brief 現在の地磁気を返す
  /// @param magnetometer_nT 地磁気ベクトルのポインタ [nT]
  void getMagnetometer(Vector3D* magnetometer_nT);

  /// @brief 現在の角加速度を返す
  /// @param gyroscope_dps 加速度のポインタ [dps]
  void getGyroscope(Vector3D* gyroscope_dps);

  /// @brief 現在の重力加速度を返す
  /// @param gravity_mps2 重力加速ベクトルのポインタ [mps2]
  void getGravityVector(Vector3D* gravity_mps2);

  /// @brief 現在の線形加速度を返す
  /// @param x 線形加速度ベクトルのポインタ [mps2]
  void getLinearAcceleration(Vector3D* linear_acceleration_mps2);

  /// @brief 現在の姿勢角を返す
  /// @param x x軸(Roll)姿勢角のポインタ [deg]
  /// @param y y軸(Pitch)姿勢角のポインタ [deg]
  /// @param z y軸(Yaw)姿勢角のポインタ [deg]
  void getOrientation(float* x, float* y, float* z);

private:
  Adafruit_BNO055 _bno;
};