#include "DetectionCounter.hpp"


/// @brief コンストラクタ
/// @param threshold 連続検知の回数(閾値)
DetectionCounter::DetectionCounter(uint32_t threshold) {
  _threshold = threshold;
}


/// @brief 状態を更新する
/// @param state 状態
/// @return 現状で何回連続検知しているか
uint32_t DetectionCounter::update(bool state) {
  // stateがtrueならカウントを増やす
  // stateがfalseならカウントを0にリセット
  if (state) {
    _count++;
  }
  else {
    _count = 0;
  }

  return _count;
}


/// @brief 検知したかを返す
/// @return true: 検知した, false: 検知していない
bool DetectionCounter::isDetected() {
  return _count >= _threshold;
}