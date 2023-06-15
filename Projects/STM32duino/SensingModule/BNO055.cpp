#include "BNO055.hpp"


void BNO055::begin() {
  _bno.begin();
  _bno.setExtCrystalUse(true);
}


void BNO055::getAcceleration(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  *x = event.acceleration.z;
  *y = event.acceleration.x;
  *z = event.acceleration.y;
}


void BNO055::getMagnetometer(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  *x = event.magnetic.z;
  *y = event.magnetic.x;
  *z = event.magnetic.y;
}


void BNO055::getGyroscope(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_GYROSCOPE);
  *x = event.gyro.z;
  *y = event.gyro.x;
  *z = event.gyro.y;
}


void BNO055::getGravityVector(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_GRAVITY);
  *x = event.acceleration.z;
  *y = event.acceleration.x;
  *z = event.acceleration.y;
}


void BNO055::getLinearAcceleration(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_LINEARACCEL);
  *x = event.acceleration.z;
  *y = event.acceleration.x;
  *z = event.acceleration.y;
}


void BNO055::getOrientation(float* x, float* y, float* z) {
  sensors_event_t event;
  _bno.getEvent(&event, Adafruit_BNO055::VECTOR_EULER);
  *x = -event.orientation.z;
  *y = -event.orientation.x;
  *z = -event.orientation.y;
}