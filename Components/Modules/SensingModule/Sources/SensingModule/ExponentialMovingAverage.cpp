#include "ExponentialMovingAverage.hpp"


/// @brief コンストラクタ
/// @param sensitivity 平滑化係数 0~1で小さいほど平滑化の強度が強い
ExponentialMovingAverage::ExponentialMovingAverage(float sensitivity) {
  _sensitivity = sensitivity;
}


/// @brief 初期化する
void ExponentialMovingAverage::clear() {
  _previousAverage = 0;
}


/// @brief 更新する
/// @param value 追加する値
void ExponentialMovingAverage::update(float value) {
  if (_previousAverage == 0) {
    _previousAverage = value;
  }

  _previousAverage = _previousAverage + _sensitivity * (value - _previousAverage);
}


/// @brief 現在の算出値を返す
/// @return 現在の算出値
float ExponentialMovingAverage::getAverage() {
  return _previousAverage;
}