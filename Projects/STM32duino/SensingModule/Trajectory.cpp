#include "Trajectory.hpp"


Trajectory::Trajectory(float sensitivityStrong, float sensitivityWeak) {
  _altitudeAverageStrong = new ExponentialMovingAverage(sensitivityStrong);
  _altitudeAverageWeak = new ExponentialMovingAverage(sensitivityWeak);
}


void Trajectory::setReferencePressure(float referencePressure) {
  _referencePressure = referencePressure;
}


float Trajectory::updateAltitude(float pressure, float temperature) {
  float altitude = (((pow((_referencePressure / pressure), (1.0 / 5.257))) - 1.0) * (temperature + 273.15)) / 0.0065;

  _altitudeAverageWeak->update(altitude);
  _altitudeAverageStrong->update(altitude);
}


float Trajectory::climbIndex() {
  return _altitudeAverageWeak->getAverage() - _altitudeAverageStrong->getAverage();
}


bool Trajectory::isFalling() {
  return climbIndex() <= 0;
}