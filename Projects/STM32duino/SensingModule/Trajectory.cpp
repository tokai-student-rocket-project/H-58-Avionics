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
/// @return 算出した高度 [m]
float Trajectory::update(float pressure, float temperature) {
  // 高度を算出する
  float altitude = (((pow((_referencePressure / pressure), (1.0 / 5.257))) - 1.0) * (temperature + 273.15)) / 0.0065;

  // 指数移動平均を更新する
  _altitudeAverageWeak->update(altitude);
  _altitudeAverageStrong->update(altitude);

  return altitude;
}


/// @brief 既知の高度から更新
/// @param altitude 高度 [m]
/// @return 高度 [m]
float Trajectory::update(float altitude) {
  // 指数移動平均を更新する
  _altitudeAverageWeak->update(altitude);
  _altitudeAverageStrong->update(altitude);

  return altitude;
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