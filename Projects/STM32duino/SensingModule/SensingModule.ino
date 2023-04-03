#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <TaskManager.h>

Adafruit_BNO055 bno = Adafruit_BNO055(-1, 0x28, &Wire);

double acceleration_x, acceleration_y, acceleration_z;

void setup() {
  Serial.begin(250000);

  Wire.setSDA(PB_7);
  Wire.setSCL(PB_6);
  Wire.begin();
  Wire.setClock(400000);
  delay(100);

  bno.begin(OPERATION_MODE_ACCONLY);
  delay(1000);

  bno.setExtCrystalUse(true);

  Tasks.add(readAcc)->startIntervalMsec(10);
}

void loop() {
  Tasks.update();
}

void readAcc() {
  sensors_event_t accelerometerData;
  bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  acceleration_x = accelerometerData.acceleration.x;
  acceleration_y = accelerometerData.acceleration.y;
  acceleration_z = accelerometerData.acceleration.z;

  Serial.print(acceleration_x); Serial.print(",");
  Serial.print(acceleration_y); Serial.print(",");
  Serial.println(acceleration_z);
}