#include "BNO055.hpp"


void BNO055::begin() {
  _bno.begin();
  _bno.setExtCrystalUse(true);
}


void BNO055::getAcceleration(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  *x = event.acceleration.x;
  *y = event.acceleration.y;
  *z = event.acceleration.z;
}


void BNO055::getMagnetometer(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  *x = event.magnetic.x;
  *y = event.magnetic.y;
  *z = event.magnetic.z;
}


void BNO055::getGyroscope(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_GYROSCOPE);
  *x = event.gyro.x;
  *y = event.gyro.y;
  *z = event.gyro.z;
}


void BNO055::getGravityVector(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_GRAVITY);
  *x = event.acceleration.x;
  *y = event.acceleration.y;
  *z = event.acceleration.z;
}


void BNO055::getLinearAcceleration(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_LINEARACCEL);
  *x = event.acceleration.x;
  *y = event.acceleration.y;
  *z = event.acceleration.z;
}


void BNO055::getOrientation(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_EULER);
  *x = event.orientation.x;
  *y = event.orientation.y;
  *z = event.orientation.z;
}