#include <TaskManager.h>
#include "BME280.hpp"


void task();
BME bme;

float referencePressure = 1013.25;
float temperature = 15;
float pressure;
float altitude;


void setup() {
  Serial.begin(115200);
  delay(800);


  Wire.setSDA(D4);
  Wire.setSCL(D5);
  Wire.begin();
  Wire.setClock(400000);

  bme.begin();


  Tasks.add(task)->startFps(182);
}


void loop() {
  Tasks.update();
}


void task() {
  bme.getPressure(&pressure);
  altitude = (((pow((referencePressure / pressure), (1.0 / 5.257))) - 1.0) * (temperature + 273.15)) / 0.0065;

  Serial.println(altitude, 6);
}