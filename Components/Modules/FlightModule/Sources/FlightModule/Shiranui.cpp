#include "Shiranui.hpp"


/// @brief コンストラクタ
/// @param pinNumber ピン番号
/// @param identify 認識用の一意な名前
Shiranui::Shiranui(uint8_t pinNumber, String identify) {
  _pin = new OutputPin(pinNumber);
  _identify = identify;

  // 事前にピンをOFFにするタスクを登録しておく
  Tasks.add(_identify, [&]() {_pin->low();});
}


/// @brief 分離信号を出力する
void Shiranui::separate() {
  _pin->high();
  // 3秒後にピンをOFFにするタスクを実行
  Tasks[_identify]->startOnceAfterSec(3);
}


/// @brief 現在の出力を返す
/// @return 現在の出力
bool Shiranui::get() {
  return _pin->get();
}