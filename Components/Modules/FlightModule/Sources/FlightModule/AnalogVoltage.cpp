#include "AnalogVoltage.hpp"


/// @brief コンストラクタ
/// @param pinNumber アナログピンのピン番号
AnalogVoltage::AnalogVoltage(uint8_t pinNumber) {
  _pinNumber = pinNumber;
  pinMode(_pinNumber, INPUT_ANALOG);
}


/// @brief 初期化する
/// @param resistanceUpper 分圧回路の上流側の抵抗値
/// @param resistanceLower 分圧回路の下流側の抵抗値
/// @return 降圧した電圧から元の電圧に戻すための補正値
float AnalogVoltage::initialize(float resistanceUpper, float resistanceLower) {
  pinMode(_pinNumber, INPUT_ANALOG);

  // 分圧回路で降圧した電圧を測定する仕様
  // 降圧後の電圧がAREFに余裕を持って収まるように調整する
  _voltageFactor = (resistanceUpper + resistanceLower) / resistanceLower;
  return _voltageFactor;
}


/// @brief 電圧を測定する
/// @return 電圧 [V]
float AnalogVoltage::voltage() {
  // 電圧 = 測定値 / 2^分解能 * AREF 
  float voltage = (float)analogRead(_pinNumber) / 4095.0 * 3.3;

  // 測定した電圧は分圧回路で降圧した後の電圧
  // 抵抗値から算出した補正値を掛けることで降圧前の電圧に戻す
  return voltage * _voltageFactor;
}