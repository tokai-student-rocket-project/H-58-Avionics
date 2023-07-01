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
  // 1
  beep4();
  sleep8();
  beep8();
  sleep8();
  beep8();
  beep4();

  // 2
  sleep4();
  beep2();
  beep4();

  // 3
  beep4();
  sleep8();
  beep8();
  sleep8();
  beep8();
  beep4();

  // 4
  sleep4();
  beep2();
  beep4();

  // 5
  beep8();
  sleep8();
  beep8();
  beep8();
  beep4();
  beep4();

  // 6
  beep8();
  sleep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 7
  beep8();
  sleep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 8
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 9
  beep8();
  sleep8();
  beep8();
  beep8();
  beep4();
  beep4();

  // 10
  beep8();
  sleep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 11
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 12
  beep8();
  beep8();
  beep8();
  beep8();
  beep4();
  sleep4();

  // 13
  beep8();
  sleep8();
  beep8();
  beep8();
  beep4();
  beep4();

  // 14
  beep8();
  sleep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 15
  beep8();
  sleep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 16
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 17
  beep8();
  sleep8();
  beep8();
  beep8();
  beep4();
  beep4();

  // 18
  beep8();
  sleep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 19
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();
  beep8();

  // 20
  beep8();
  beep8();
  beep8();
  beep8();
  beep4();
  sleep4();

  // 21
  beep2();
  beep4();
  beep4();

  // 22
  beep4();
  beep4();
  beep4();
  sleep4();

  // 23
  beep8();
  beep8();
  beep4();
  beep4();
  beep4();

  // 24
  beep8();
  beep8();
  beep4();
  beep4();
  sleep4();

  // 25
  beep2();
  beep4();
  beep4();

  // 26
  beep4();
  beep4();
  beep4();
  sleep4();

  // 27
  beep4();
  beep4();
  beep4();
  beep4();

  // 28
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
