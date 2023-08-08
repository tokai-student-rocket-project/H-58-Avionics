#pragma once
#include <Arduino.h>
#include "ExponentialMovingAverage.hpp"


/// @brief 高度算出と落下検知用のクラス
class Trajectory {
public:
  /// @brief コンストラクタ
  /// @param sensitivityStrong 小さい(強い)方の平滑化定数
  /// @param sensitivityWeak 大きい(弱い)方の平滑化定数
  Trajectory(float sensitivityStrong = 0.25, float sensitivityWeak = 0.75);

  /// @brief 参照気圧を更新する
  /// @param referencePressure 参照気圧 [hPa]
  void setReferencePressure(float referencePressure);

  /// @brief 現在の参照気圧を返す
  /// @return 参照気圧 [hPa]
  float getReferencePressure();


  /// @brief 気圧と気温から高度を算出して更新
  /// @param pressure 気圧 [hPa]
  /// @param temperature 気温 [degC]
  void update(float pressure, float temperature);

  /// @brief 既知の高度から更新
  /// @param altitude 高度 [m]
  void update(float altitude);

  /// @brief 算出済みの高度を返す
  /// @return 高度 [m]
  float getAltitude();

  /// @brief 算出済みの上昇率を返す
  /// @return 上昇率 [m/s]
  float getClimbRate();

  /// @brief 落下検知用に算出した上昇指数を返す
  /// @return 上昇指数
  float climbIndex();

  /// @brief 落下を検知しているかを返す
  /// @return true: 落下中, false: 落下中でない
  bool isFalling();

private:
  ExponentialMovingAverage* _altitudeAverageStrong;
  ExponentialMovingAverage* _altitudeAverageWeak;

  float _referencePressure = 1013.25;

  float _latestAltitude;
  float _latestClimbRate;

  float _lastAltitude;
  float _lastTime;
};