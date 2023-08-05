#include "Trajectory.hpp"


/// @brief コンストラクタ
/// @param sensitivityStrong 小さい(強い)方の平滑化定数
/// @param sensitivityWeak 大きい(弱い)方の平滑化定数
Trajectory::Trajectory(float sensitivityStrong, float sensitivityWeak) {
  _altitudeAverageStrong = new ExponentialMovingAverage(sensitivityStrong);
  _altitudeAverageWeak = new ExponentialMovingAverage(sensitivityWeak);
}


/// @brief 参照気圧を更新する
/// @param referencePressure 参照気圧 [hPa]
void Trajectory::setReferencePressure(float referencePressure) {
  _referencePressure = referencePressure;
}


/// @brief 現在の参照気圧を返す
/// @return 参照気圧 [hPa]
float Trajectory::getReferencePressure() {
  return _referencePressure;
}


/// @brief 気圧と気温から高度を算出して更新
/// @param pressure 気圧 [hPa]
/// @param temperature 気温 [degC]
void Trajectory::update(float pressure, float temperature) {
  // 高度を算出する
  float altitude = (((pow((_referencePressure / pressure), (1.0 / 5.257))) - 1.0) * (temperature + 273.15)) / 0.0065;
  update(altitude);
}


/// @brief 既知の高度から更新
/// @param altitude 高度 [m]
void Trajectory::update(float altitude) {
  _latestAltitude = altitude;

  // 指数移動平均を更新する
  _altitudeAverageWeak->update(altitude);
  _altitudeAverageStrong->update(altitude);

  // 上昇率を算出する
  float time = (float)millis() / 1000.0;
  float altitudeDifference = _latestAltitude - _lastAltitude;
  float timeDifference = time - _lastTime;

  if (altitudeDifference == 0.0 || timeDifference == 0.0) return;

  // 高度を時間で微分
  _latestClimbRate = altitudeDifference / timeDifference;
  _lastAltitude = _latestAltitude;
  _lastTime = time;
}


/// @brief 算出済みの高度を返す
/// @return 高度 [m]
float Trajectory::getAltitude() {
  return _latestAltitude;
}


/// @brief 算出済みの上昇率を返す
/// @return 上昇率 [m/s]
float Trajectory::getClimbRate() {
  return _latestClimbRate;
}


/// @brief 落下検知用に算出した上昇指数を返す
/// @return 上昇指数
float Trajectory::climbIndex() {
  return _altitudeAverageWeak->getAverage() - _altitudeAverageStrong->getAverage();
}


/// @brief 落下を検知しているかを返す
/// @return true: 落下中, false: 落下中でない
bool Trajectory::isFalling() {
  // 上昇指数が0以下なら落下判定
  return climbIndex() <= 0;
}