#include "ADXL375.hpp"


void ADXL375::begin() {
  // Test connection
  // Wire.beginTransmission(0xA7);
  // Wire.write(0x00); // DEVID
  // Wire.write(0x00);
  // Wire.endTransmission();
  // Wire.requestFrom(0xA7, 1);
  // while (Wire.available() < 1);
  // Serial.println(Wire.read(), BIN);

  if (!accel.begin())
  {
    /* There was a problem detecting the ADXL375 ... check your connections */
    Serial.println("Ooops, no ADXL375 detected ... Check your wiring!");
    while (1);
  }

  accel.setDataRate(ADXL3XX_DATARATE_1600_HZ);
}


float ADXL375::get() {
  sensors_event_t event;
  accel.getEvent(&event);

  return event.acceleration.x;
}