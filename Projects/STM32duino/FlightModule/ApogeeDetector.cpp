#include "ApogeeDetector.hpp"


ApogeeDetector::ApogeeDetector(float sensitivityStrong, float sensitivityWeak) {
  _altitudeAverageStrong = new ExponentialMovingAverage(sensitivityStrong);
  _altitudeAverageWeak = new ExponentialMovingAverage(sensitivityWeak);
}


void ApogeeDetector::update(float altitude) {
  _altitudeAverageWeak->update(altitude);
  _altitudeAverageStrong->update(altitude);

  Serial.print(altitude);
  Serial.print(",");
  Serial.print(_altitudeAverageWeak->getAverage());
  Serial.print(",");
  Serial.println(_altitudeAverageStrong->getAverage());
}


bool ApogeeDetector::isDetected() {
  float difference = _altitudeAverageWeak->getAverage() - _altitudeAverageStrong->getAverage();
  return difference <= 0;
}