#pragma once


#include <Arduino.h>


/// @brief 複数回連続検知用のクラス
class DetectionCounter {
public:
  /// @brief コンストラクタ
  /// @param threshold 連続検知の回数(閾値)
  DetectionCounter(uint32_t threshold);


  /// @brief 状態を更新する
  /// @param state 状態
  /// @return 現状で何回連続検知しているか
  uint32_t update(bool state);

  /// @brief 検知したかを返す
  /// @return true: 検知した, false: 検知していない
  bool isDetected();

private:
  uint32_t _threshold;
  uint32_t _count;
};