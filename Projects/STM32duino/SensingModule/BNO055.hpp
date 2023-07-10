#pragma once


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

/// @brief BME055の抽象化クラス
class BNO055 {
public:
  /// @brief BME280の抽象化クラス
  void begin();


  /// @brief 現在の加速度ベクトルを返す
  /// @param x x軸加速度のポインタ [mps2]
  /// @param y y軸加速度のポインタ [mps2]
  /// @param z y軸加速度のポインタ [mps2]
  void getAcceleration(float* x, float* y, float* z);

  /// @brief 現在の地磁気を返す
  /// @param x x軸地磁気のポインタ [nT]
  /// @param y y軸地磁気のポインタ [nT]
  /// @param z y軸地磁気のポインタ [nT]
  void getMagnetometer(float* x, float* y, float* z);


  /// @brief 現在の角加速度を返す
  /// @param x x軸周り角加速度のポインタ [dps]
  /// @param y y軸周り角加速度のポインタ [dps]
  /// @param z y軸周り角加速度のポインタ [dps]
  void getGyroscope(float* x, float* y, float* z);

  /// @brief 現在の重力加速度を返す
  /// @param x x軸重力加速度のポインタ [mps2]
  /// @param y y軸重力加速度のポインタ [mps2]
  /// @param z y軸重力加速度のポインタ [mps2]
  void getGravityVector(float* x, float* y, float* z);

  /// @brief 現在の線形加速度を返す
  /// @param x x軸線形加速度のポインタ [mps2]
  /// @param y y軸線形加速度のポインタ [mps2]
  /// @param z y軸線形加速度のポインタ [mps2]
  void getLinearAcceleration(float* x, float* y, float* z);

  /// @brief 現在の姿勢角を返す
  /// @param x x軸(Roll)姿勢角のポインタ [deg]
  /// @param y y軸(Pitch)姿勢角のポインタ [deg]
  /// @param z y軸(Yaw)姿勢角のポインタ [deg]
  void getOrientation(float* x, float* y, float* z);

private:
  Adafruit_BNO055 _bno;
};