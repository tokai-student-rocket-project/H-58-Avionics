#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_I2CDevice.h>
#include "Adafruit_BNO055.h"
#include <TaskManager.h>

Adafruit_BNO055 bno = Adafruit_BNO055();

double acceleration_x, acceleration_y, acceleration_z;

bool write8(Adafruit_BNO055::adafruit_bno055_reg_t reg, byte value) {
  Wire.beginTransmission(0x28);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
  return true;
}

byte read8(Adafruit_BNO055::adafruit_bno055_reg_t reg) {
  Wire.beginTransmission(0x28);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom(0x28, 1);
  return (byte)Wire.read();
}

void setMode(adafruit_bno055_opmode_t mode) {
  bno._mode = mode;
  write8((Adafruit_BNO055::adafruit_bno055_reg_t)0x3D, mode);
  delay(30);
}

bool begin(adafruit_bno055_opmode_t mode) {
  /* Make sure we have the right device */
  uint8_t id = read8((Adafruit_BNO055::adafruit_bno055_reg_t)0x00);
  if (id != BNO055_ID) {
    delay(1000); // hold on for boot
    id = read8((Adafruit_BNO055::adafruit_bno055_reg_t)0x00);
    if (id != BNO055_ID) {
      return false; // still not? ok bail
    }
  }

  /* Switch to config mode (just in case since this is the default) */
  setMode(OPERATION_MODE_CONFIG);

  /* Reset */
  write8((Adafruit_BNO055::adafruit_bno055_reg_t)0x3F, 0x20);
  /* Delay incrased to 30ms due to power issues https://tinyurl.com/y375z699 */
  delay(30);
  while (read8((Adafruit_BNO055::adafruit_bno055_reg_t)0x00) != BNO055_ID) {
    delay(10);
  }
  delay(50);

  /* Set to normal power mode */
  write8((Adafruit_BNO055::adafruit_bno055_reg_t)0x3E, 0x00);
  delay(10);

  write8((Adafruit_BNO055::adafruit_bno055_reg_t)0x07, 0);

  write8((Adafruit_BNO055::adafruit_bno055_reg_t)0x3F, 0x0);
  delay(10);
  /* Set the requested operating mode (see section 3.3) */
  setMode(mode);
  delay(20);

  return true;
}

void setup() {
  Serial.begin(115200);

  Wire.setSDA(PB_7);
  Wire.setSCL(PB_6);
  Wire.begin();
  Wire.setClock(400000);
  delay(100);

  begin(OPERATION_MODE_ACCONLY);
  delay(1000);

  bno.setExtCrystalUse(true);

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