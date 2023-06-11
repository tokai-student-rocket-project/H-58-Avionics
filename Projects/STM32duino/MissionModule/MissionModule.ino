#include <TaskManager.h>
#include "ADXL375.hpp"


namespace timer {
  uint32_t referenceTime;

  void task1kHz();
}

namespace sensor {
  ADXL375 adxl;
}

namespace data {
  float acceleration_x, acceleration_y, acceleration_z;
}


void setup() {
  Wire.begin();
  Wire.setClock(400000);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);
  while (!Serial);

  Serial.println("start");

  sensor::adxl.begin();

  digitalWrite(LED_BUILTIN, HIGH);

  Tasks.add(timer::task1kHz)->startIntervalMsec(1);
}


void loop() {
  Tasks.update();
}


void timer::task1kHz() {
  // Serial.print("x:"); Serial.print(data::acceleration_x / 9.8);
  // Serial.print(",y:"); Serial.print(data::acceleration_y / 9.8);
  // Serial.print(",z:"); Serial.println(data::acceleration_z / 9.8);

  Serial.println(sensor::adxl.get());

  uint32_t time = micros();
  // Serial.println(1000000.0 / (float)(time - timer::referenceTime), 2);
  timer::referenceTime = time;
}