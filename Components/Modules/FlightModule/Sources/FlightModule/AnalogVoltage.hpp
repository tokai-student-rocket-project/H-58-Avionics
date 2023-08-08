#pragma once


#include <Arduino.h>


/// @brief アナログピンで電圧を測定するクラス
class AnalogVoltage {
public:
  /// @brief コンストラクタ
  /// @param pinNumber アナログピンのピン番号
  AnalogVoltage(uint8_t pinNumber);


  /// @brief 初期化する
  /// @param resistanceUpper 分圧回路の上流側の抵抗値
  /// @param resistanceLower 分圧回路の下流側の抵抗値
  /// @return 降圧した電圧から元の電圧に戻すための補正値
  float initialize(float resistanceUpper, float resistanceLower);

  /// @brief 電圧を測定する
  /// @return 電圧 [V]
  float voltage();

private:
  uint8_t _pinNumber;
  float _voltageFactor;
};