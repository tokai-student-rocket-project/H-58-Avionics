#include <Wire.h>
#include <TaskManager.h>
#include "BNO055.hpp"

BNO055 bno055;

double acceleration_x, acceleration_y, acceleration_z;

void setup() {
  Serial.begin(115200);

  Wire.setSDA(PB_7);
  Wire.setSCL(PB_6);
  Wire.begin();
  Wire.setClock(400000);
  delay(100);

  bno055.initialize();

  Tasks.add(task100Hz)->startIntervalMsec(10);
}

void loop() {
  Tasks.update();
}

void task100Hz() {
  bno055.getAcceleration(&acceleration_x, &acceleration_y, &acceleration_z);

  Serial.print(acceleration_x); Serial.print(",");
  Serial.print(acceleration_y); Serial.print(",");
  Serial.println(acceleration_z);
}