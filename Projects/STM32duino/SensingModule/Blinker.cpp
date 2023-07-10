#include "Blinker.hpp"


/// @brief コンストラクタ
/// @param pinNumber ピン番号
/// @param identify 内部でタスクを処理するための一意な名前
Blinker::Blinker(uint8_t pinNumber, String identify) {
  _pin = new OutputPin(pinNumber);
  _identify = identify;

  // ピンの状態を切り替えるタスクを登録しておく
  Tasks.add(_identify, [&]() {_pin->toggle();});
}


/// @brief 点滅を開始する
/// @param fps 1秒間に何回点滅するか
void Blinker::startBlink(float fps) {
  // 2回切り替えてOFF->ON->OFFなので2をかける
  Tasks[_identify]->startFps(fps * 2.0);
}


/// @brief 点滅を終了する
void Blinker::stopBlink() {
  Tasks[_identify]->stop();
}


/// @brief 点滅とは関係なくONにしたいとき用
void Blinker::on() {
  _pin->on();
}


/// @brief 点滅とは関係なくOFFにしたいとき用
void Blinker::off() {
  _pin->off();
}