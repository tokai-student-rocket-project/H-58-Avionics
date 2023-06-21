#include "Shiranui.hpp"


Shiranui::Shiranui(uint8_t pinNumber, String identify) {
  _pin = new OutputPin(pinNumber);
  _identify = identify;

  Tasks.add(_identify, [&]() {_pin->off();});
}


void Shiranui::separate() {
  _pin->on();
  Tasks[_identify]->startOnceAfterSec(3);
}


bool Shiranui::get() {
  return _pin->get();
}