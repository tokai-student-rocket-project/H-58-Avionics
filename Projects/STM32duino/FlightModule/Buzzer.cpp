#include "Buzzer.hpp"


Buzzer::Buzzer(uint8_t pinNumber, String identify) {
  _pin = new OutputPin(pinNumber);
  _identify = identify;

  Tasks.add(_identify, [&]() {_pin->toggle();});
}


void Buzzer::beepOnce() {
  Tasks[_identify]->startIntervalMsecForCount(100, 2);
}


void Buzzer::beepTwice() {
  Tasks[_identify]->startIntervalMsecForCount(100, 4);
}

void Buzzer::beepLongOnce() {
  Tasks[_identify]->startIntervalMsecForCount(400, 2);
}


void Buzzer::electricalParade() {
  // 1小節
  beep4();
  sleep8();
  beep8();
  sleep8();
  beep8();
  beep4();

  // 2小節
  sleep4();
  beep2();
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
  beep2();
  beep4();

  // 5小節
  beep8();
  sleep8();
  beep8();
  beep8();
  beep4();
  beep4();

  // 6小節
  beep8();
  sleep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 7小節
  beep8();
  sleep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 8小節
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 9小節
  beep8();
  sleep8();
  beep8();
  beep8();
  beep4();
  beep4();

  // 10小節
  beep8();
  sleep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 11小節
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 12小節
  beep8();
  beep8();
  beep8();
  beep8();
  beep4();
  sleep4();

  // 13小節
  beep8();
  sleep8();
  beep8();
  beep8();
  beep4();
  beep4();

  // 14小節
  beep8();
  sleep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 15小節
  beep8();
  sleep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 16小節
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 17小節
  beep8();
  sleep8();
  beep8();
  beep8();
  beep4();
  beep4();

  // 18小節
  beep8();
  sleep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 19小節
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 20小節
  beep8();
  beep8();
  beep8();
  beep8();
  beep4();
  sleep4();

  // 21小節
  beep2();
  beep4();
  beep4();

  // 22小節
  beep4();
  beep4();
  beep4();
  sleep4();

  // 23小節
  beep8();
  beep8();
  beep4();
  beep4();
  beep4();

  // 24小節
  beep8();
  beep8();
  beep4();
  beep4();
  sleep4();

  // 25小節
  beep2();
  beep4();
  beep4();

  // 26小節
  beep4();
  beep4();
  beep4();
  sleep4();

  // 27小節
  beep4();
  beep4();
  beep4();
  beep4();

  // 28小節
  beep8();
  beep8();
  beep4();
  beep4();
  sleep4();
}


void Buzzer::beep2() {
  _pin->on();
  delay(500);
  _pin->off();
  delay(15);
}

void Buzzer::beep4() {
  _pin->on();
  delay(250);
  _pin->off();
  delay(15);
}

void Buzzer::beep8() {
  _pin->on();
  delay(125);
  _pin->off();
  delay(15);
}

void Buzzer::sleep4() {
  delay(250);
  delay(15);
}

void Buzzer::sleep8() {
  delay(125);
  delay(15);
}
