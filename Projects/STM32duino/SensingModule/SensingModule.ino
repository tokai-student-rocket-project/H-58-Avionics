#include <Wire.h>
#include <TaskManager.h>

int16_t acceleration_x_raw, acceleration_y_raw, acceleration_z_raw;
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

  // OPR_MODEをNDOFに変更
  write8(0x3D, 0x0C);
  delay(50);

  Tasks.add(task100Hz)->startIntervalMsec(10);
}

void loop() {
  Tasks.update();
}

void task100Hz() {
  Wire.beginTransmission(0x28);
  Wire.write(0x08);
  Wire.endTransmission();
  Wire.requestFrom(0x28, 6);

  acceleration_x_raw = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);
  acceleration_y_raw = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);
  acceleration_z_raw = ((int16_t)Wire.read()) | (((int16_t)Wire.read()) << 8);

  acceleration_x = ((double)acceleration_x_raw) / 100.0;
  acceleration_y = ((double)acceleration_y_raw) / 100.0;
  acceleration_z = ((double)acceleration_z_raw) / 100.0;

  Serial.print(acceleration_x); Serial.print(",");
  Serial.print(acceleration_y); Serial.print(",");
  Serial.println(acceleration_z);
}