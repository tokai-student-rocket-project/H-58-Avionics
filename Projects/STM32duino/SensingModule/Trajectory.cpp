#include "Trajectory.hpp"


Trajectory::Trajectory(float sensitivityStrong, float sensitivityWeak) {
  _altitudeAverageStrong = new ExponentialMovingAverage(sensitivityStrong);
  _altitudeAverageWeak = new ExponentialMovingAverage(sensitivityWeak);
}


void Trajectory::update(float altitude) {
  _altitudeAverageWeak->update(altitude);
  _altitudeAverageStrong->update(altitude);
}


float Trajectory::climbIndex() {
  return _altitudeAverageWeak->getAverage() - _altitudeAverageStrong->getAverage();
}


bool Trajectory::isFalling() {
  return climbIndex() <= 0;
}