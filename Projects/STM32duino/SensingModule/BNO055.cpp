#include "BNO055.hpp"


bool BNO055::begin() {
  bool isSucceeded = _bno.begin();
  _bno.setExtCrystalUse(true);

  return isSucceeded;
}


String BNO055::getMode() {
  switch (_bno.getMode()) {
  case 12:
    return "NDOF";
    break;
  }
}


String BNO055::getSystemStatus() {
  uint8_t systemStatus;
  uint8_t selfTestResult;
  uint8_t systemError;

  _bno.getSystemStatus(&systemStatus, &selfTestResult, &systemError);

  switch (systemStatus) {
  case 0:
    return "Idle";
    break;
  case 1:
    return "System Error";
    break;
  case 2:
    return "Initializing Peripherals";
    break;
  case 3:
    return "System Iniitalization";
    break;
  case 4:
    return "Executing Self-Test";
    break;
  case 5:
    return "Sensor fusio algorithm running";
    break;
  case 6:
    return "System running without fusion algorithms";
    break;
  }
}


String BNO055::getSelfTestResult() {
  uint8_t systemStatus;
  uint8_t selfTestResult;
  uint8_t systemError;

  _bno.getSystemStatus(&systemStatus, &selfTestResult, &systemError);

  switch (selfTestResult) {
  case 0x0F:
    return "All Passed";
    break;
  default:
    return "Failed";
    break;
  }
}


String BNO055::getSystemError() {
  uint8_t systemStatus;
  uint8_t selfTestResult;
  uint8_t systemError;

  _bno.getSystemStatus(&systemStatus, &selfTestResult, &systemError);

  switch (systemError) {
  case 0:
    return "No error";
    break;
  case 1:
    return "Peripheral initialization error";
    break;
  case 2:
    return "System initialization error";
    break;
  case 3:
    return "Self test result failed";
    break;
  case 4:
    return "Register map value out of range";
    break;
  case 5:
    return "Register map address out of range";
    break;
  case 6:
    return "Register map write error";
    break;
  case 7:
    return "BNO low power mode not available for selected operat ion mode";
    break;
  case 8:
    return "Accelerometer power mode not available";
    break;
  case 9:
    return "Fusion algorithm configuration error";
    break;
  case 10:
    return "Sensor configuration error";
    break;
  }
}


bool BNO055::isFullyCalibrated() {
  return _bno.isFullyCalibrated();
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