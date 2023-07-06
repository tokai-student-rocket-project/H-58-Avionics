#include "GNSS.hpp"


void GNSS::begin() {
  Wire.begin();

  _gnss.begin();
  _gnss.setI2COutput(COM_TYPE_UBX);
  _gnss.setNavigationFrequency(10);
  _gnss.setAutoPVT(true);
}


bool GNSS::available() {
  if (!_gnss.getPVT() || _gnss.getInvalidLlh()) {
    return false;
  }

  int32_t latitude = _gnss.getLatitude();
  int32_t longitude = _gnss.getLongitude();

  return latitude != 0 && longitude != 0;
}


float GNSS::getLatitude() {
  return (float)_gnss.getLatitude() / 10000000.0;
}


float GNSS::getLongitude() {
  return (float)_gnss.getLongitude() / 10000000.0;
}

