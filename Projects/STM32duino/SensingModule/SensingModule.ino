#include <Wire.h>
#include <TaskManager.h>

int16_t rax, ray, raz;
float ax, ay, az;

void setup() {
  Serial.begin(115200);

  Wire.setSDA(PB_7);
  Wire.setSCL(PB_6);
  Wire.begin();
  Wire.setClock(400000);

  // OPR_MODEをCONFIGMODEに変更
  Wire.beginTransmission(0x28);
  Wire.write(0x3D);
  Wire.write(0x00);
  Wire.endTransmission();

  // SYS_TRIGGERのRST_SYSをトリガー
  Wire.beginTransmission(0x28);
  Wire.write(0x3F);
  Wire.write(0x20);
  Wire.endTransmission();

  // PWR_MODEをNORMALに変更
  Wire.beginTransmission(0x28);
  Wire.write(0x3E);
  Wire.write(0x00);
  Wire.endTransmission();

  // PAGEを0に変更
  Wire.beginTransmission(0x28);
  Wire.write(0x07);
  Wire.write(0x00);
  Wire.endTransmission();

  // SYS_TRIGGERをリセット
  Wire.beginTransmission(0x28);
  Wire.write(0x3F);
  Wire.write(0x00);
  Wire.endTransmission();

  // OPR_MODEをNDOFに変更
  Wire.beginTransmission(0x28);
  Wire.write(0x3F);
  Wire.write(0x0C);
  Wire.endTransmission();

  Tasks.add(readAcc)->startIntervalMsec(10);
}

void loop() {
  Tasks.update();
}

void readAcc() {
  Wire.beginTransmission(0x28);
  Wire.write(0x08);
  Wire.endTransmission();
  Wire.requestFrom(0x28, 2);
  rax = (int16_t)(Wire.read() << 8) | (int8_t)Wire.read();

  Wire.beginTransmission(0x28);
  Wire.write(0x0A);
  Wire.endTransmission();
  Wire.requestFrom(0x28, 2);
  ray = (int16_t)(Wire.read() << 8) | (int8_t)Wire.read();

  Wire.beginTransmission(0x28);
  Wire.write(0x0C);
  Wire.endTransmission();
  Wire.requestFrom(0x28, 2);
  raz = (int16_t)(Wire.read() << 8) | (int8_t)Wire.read();

  ax = (float)rax / 16384.0 * 100;
  ay = (float)ray / 16384.0 * 100;
  az = (float)raz / 16384.0 * 100;

  Serial.print(ax);
  Serial.print(",");
  Serial.print(ay);
  Serial.print(",");
  Serial.println(az);
}