#include "Blinker.hpp"


Blinker::Blinker(uint8_t pinNumber, String identify) {
  _pin = new OutputPin(pinNumber);
  _identify = identify;

  Tasks.add(_identify, [&]() {_pin->toggle();});
}


void Blinker::startBlink(float fps) {
  Tasks[_identify]->startFps(fps);
}


void Blinker::stopBlink() {
  Tasks[_identify]->stop();
}


void Blinker::on() {
  _pin->high();
}


void Blinker::off() {
  _pin->low();
}