#include "Buzzer.hpp"


/// @brief コンストラクタ
/// @param pinNumber ピン番号
/// @param identify 認識用の一意な名前
Buzzer::Buzzer(uint8_t pinNumber, String identify) {
  _pin = new OutputPin(pinNumber);
  _identify = identify;

  // 事前にピンの状態をトグルするタスクを登録しておく
  Tasks.add(_identify, [&]() {_pin->toggle();});
}


/// @brief 短音1回を鳴らす
void Buzzer::beepOnce() {
  _pin->low();
  Tasks[_identify]->startIntervalMsecForCount(100, 2);
}


/// @brief 短音2回を鳴らす
void Buzzer::beepTwice() {
  _pin->low();
  Tasks[_identify]->startIntervalMsecForCount(100, 4);
}


/// @brief 超音1回を鳴らす
void Buzzer::beepLongOnce() {
  _pin->low();
  Tasks[_identify]->startIntervalMsecForCount(400, 2);
}


/// @brief ほろびのうたを鳴らす ブロッキング処理なので注意
void Buzzer::electricalParade() {

  // BPM = 240

  // Intro.

  // 1小節
  // flat
  beep4(); // F
  sleep8();
  beep8();
  sleep8();
  beep8();
  beep4();

  // 2小節
  sleep4();
  beep2(); // flat
  beep4();

  // 3小節
  beep4();
  sleep8();
  beep8();
  sleep8();
  beep8();
  beep4();

  // 4小節
  sleep4();
  beep2(); // flat
  beep4();

  // A

  // 5小節
  // flat
  beep8(); // F
  sleep8();
  beep8();
  beep8();
  beep4(); // F/A
  beep4();

  // 6小節
  beep8(); // B flat
  sleep8();
  beep8();
  beep8();
  beep8(); // C
  beep8();
  beep8(); // C7/B flat
  beep8();

  // 7小節
  beep8(); // F/A
  sleep8();
  beep8();
  beep8();
  beep8(); // F
  beep8();
  beep8();
  beep8();

  // 8小節
  beep8(); // G7
  beep8();
  beep8(); // G7/D C
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 9小節
  // flat
  beep8(); // F
  sleep8();
  beep8();
  beep8();
  beep4(); // F/A
  beep4();

  // 10小節
  beep8(); // B flat
  sleep8();
  beep8();
  beep8();
  beep8(); // C
  beep8();
  beep8(); // C7/B flat
  beep8();

  // 11小節
  beep8(); // F/A
  beep8();
  beep8();
  beep8();
  beep8(); // B flat
  beep8();
  beep8();
  beep8();

  // 12小節
  beep8(); // C
  beep8();
  beep8(); // C/E
  beep8();
  beep4(); // F
  sleep4();

  // 13小節
  // flat
  beep8(); // F
  sleep8();
  beep8();
  beep8();
  beep4(); // F/A
  beep4();

  // 14小節
  beep8(); // B flat
  sleep8();
  beep8();
  beep8();
  beep8(); // C
  beep8();
  beep8(); // C7/B flat
  beep8();

  // 15小節
  beep8(); // F/A
  sleep8();
  beep8();
  beep8();
  beep8(); // F
  beep8();
  beep8();
  beep8();

  // 16小節
  beep8(); // G
  beep8();
  beep8(); // G/D
  beep8();
  beep8(); // C
  beep8();
  beep8();
  beep8();

  // 17小節
  // flat
  beep8(); // F
  sleep8();
  beep8();
  beep8();
  beep4(); // F/A
  beep4();

  // 18小節
  beep8(); // B flat
  sleep8();
  beep8();
  beep8();
  beep8(); // C
  beep8();
  beep8(); // C7/B flat
  beep8();

  // 19小節
  beep8(); // F/A
  beep8();
  beep8();
  beep8();
  beep8(); // B flat
  beep8();
  beep8();
  beep8();

  // 20小節
  beep8(); // C
  beep8();
  beep8(); // C/E
  beep8();
  beep4(); // F
  sleep4();

  // B

  // 21小節
  // flat
  beep2(); // F
  beep4(); // C/E
  beep4();

  // 22小節
  beep4(); // G/D
  beep4();
  beep4(); // C
  sleep4();

  // 23小節
  beep8(); // F
  beep8(); // natural
  beep4();
  beep4(); // C/G
  beep4();

  // 24小節
  beep8(); // F/A
  beep8();
  beep4(); // G7/B C
  beep4();
  sleep4();

  // 25小節
  // flat
  beep2(); // F
  beep4(); // C/E
  beep4();

  // 26小節
  beep4(); // G/D
  beep4();
  beep4(); // C
  sleep4();

  // 27小節
  beep4(); // F
  beep4();
  beep4(); // C/G
  beep4();

  // 28小節
  beep8(); // F/A
  beep8();
  beep4(); // G7/B C
  beep4();
  sleep4();
}


/// @brief 2分音符
void Buzzer::beep2() {
  _pin->high();
  delay(500);
  _pin->low();
  delay(15);
}


/// @brief 4分音符
void Buzzer::beep4() {
  _pin->high();
  delay(250);
  _pin->low();
  delay(15);
}


/// @brief 8分音符
void Buzzer::beep8() {
  _pin->high();
  delay(125);
  _pin->low();
  delay(15);
}


/// @brief 4分休符
void Buzzer::sleep4() {
  delay(250);
  delay(15);
}


/// @brief 8分休符
void Buzzer::sleep8() {
  delay(125);
  delay(15);
}
