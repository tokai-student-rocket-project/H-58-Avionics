#pragma once


#include <Arduino.h>


/// @brief 指数移動平均算出用のクラス
class ExponentialMovingAverage {
public:
  /// @brief コンストラクタ
  /// @param sensitivity 平滑化係数 0~1で小さいほど平滑化の強度が強い
  ExponentialMovingAverage(float sensitivity);


  /// @brief 初期化する
  void clear();

  /// @brief 更新する
  /// @param value 追加する値
  void update(float value);

  /// @brief 現在の算出値を返す
  /// @return 現在の算出値
  float getAverage();

private:
  float _sensitivity = 0;
  float _previousAverage = 0;
};