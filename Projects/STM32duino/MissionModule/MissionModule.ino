#include <TaskManager.h>
#include "ADXL375.hpp"


namespace timer {
  void task1kHz();
}

namespace sensor {
  ADXL375 adxl;
}

namespace data {
  float acceleration_x, acceleration_y, acceleration_z;
}


void setup() {
  Serial.begin(115200);

  sensor::adxl.begin();
  sensor::adxl.setOffsets();

  Tasks.add(timer::task1kHz)->startIntervalMsec(1);
}


void loop() {
  Tasks.update();
}


void timer::task1kHz() {
  sensor::adxl.getAcceleration(&data::acceleration_x, &data::acceleration_y, &data::acceleration_z);

  Serial.print("x:"); Serial.print(data::acceleration_x / 9.8);
  Serial.print(",y:"); Serial.print(data::acceleration_y / 9.8);
  Serial.print(",z:"); Serial.println(data::acceleration_z / 9.8);
}