#pragma once


#include <Arduino.h>
#include <Wire.h>


/// @brief BME055の軽量版抽象化クラス
class BNO055 {
public:
  /// @brief センサを初期化して起動する
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
  void write(uint8_t address, uint8_t data);
  void readVector3D(uint8_t address, float lsb, float* x, float* y, float* z);
};