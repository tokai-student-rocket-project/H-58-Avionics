#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_I2CDevice.h>
#include "Adafruit_BNO055.h"
#include <TaskManager.h>

Adafruit_BNO055 bno = Adafruit_BNO055();

double acceleration_x, acceleration_y, acceleration_z;

bool write8(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(0x28);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
  return true;
}

uint8_t read8(uint8_t reg) {
  Wire.beginTransmission(0x28);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(0x28, 1);
  return (uint8_t)Wire.read();
}

void setup() {
  Serial.begin(115200);

  Wire.setSDA(PB_7);
  Wire.setSCL(PB_6);
  Wire.begin();
  Wire.setClock(400000);
  delay(100);

  // OPR_MODEをCONFIGMODEに変更
  bno._mode = (adafruit_bno055_opmode_t)0x00;
  write8(0x3D, 0x00);
  delay(50);

  // PWR_MODEをNormal Modeに変更
  write8(0x3E, 0x00);
  delay(10);

  // PAGEを0に変更
  write8(0x07, 0);

  //SYS_TRIGGERのCLK_SELをトリガー
  write8(0x3F, 0x80);
  delay(10);

  // OPR_MODEをACCONLYに変更
  bno._mode = (adafruit_bno055_opmode_t)0x01;
  write8(0x3D, 0x01);
  delay(50);

  Tasks.add(task100Hz)->startIntervalMsec(10);
}

void loop() {
  Tasks.update();
}

void task100Hz() {
  sensors_event_t accelerometerData;
  bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  acceleration_x = accelerometerData.acceleration.x;
  acceleration_y = accelerometerData.acceleration.y;
  acceleration_z = accelerometerData.acceleration.z;

  Serial.print(acceleration_x); Serial.print(",");
  Serial.print(acceleration_y); Serial.print(",");
  Serial.println(acceleration_z);
}