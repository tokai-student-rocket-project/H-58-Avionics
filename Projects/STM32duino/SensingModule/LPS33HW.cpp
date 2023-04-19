#include "LPS33HW.hpp"


void LPS33HW::begin() {
  _lps.begin_I2C();
  _lps.setDataRate(LPS22_RATE_75_HZ);
}


void LPS33HW::getPressure(float* pressure) {
  sensors_event_t temp_event;
  sensors_event_t pressure_event;
  _lps.getEvent(&pressure_event, &temp_event);
  *pressure = pressure_event.pressure;
}