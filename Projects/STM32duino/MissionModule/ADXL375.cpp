#include "ADXL375.hpp"


void ADXL375::begin() {
  // Disable interrupts to start
  Wire.beginTransmission(0x53);
  Wire.write(0x2E);
  Wire.write(0);
  Wire.endTransmission();

  // 62.5 mg/LSB (so 0xFF = 16 g)
  Wire.beginTransmission(0x53);
  Wire.write(0x1D);
  Wire.write(20);
  Wire.endTransmission();

  // Max tap duration, 625 µs/LSB
  Wire.beginTransmission(0x53);
  Wire.write(0x21);
  Wire.write(50);
  Wire.endTransmission();

  // Tap latency, 1.25 ms/LSB, 0=no double tap
  Wire.beginTransmission(0x53);
  Wire.write(0x22);
  Wire.write(0);
  Wire.endTransmission();

  // Waiting period,  1.25 ms/LSB, 0=no double tap
  Wire.beginTransmission(0x53);
  Wire.write(0x23);
  Wire.write(0);
  Wire.endTransmission();

  // Enable the XYZ axis for tap
  Wire.beginTransmission(0x53);
  Wire.write(0x2A);
  Wire.write(0x7);
  Wire.endTransmission();

  // Enable measurements
  Wire.beginTransmission(0x53);
  Wire.write(0x2D);
  Wire.write(0x08);
  Wire.endTransmission();

  // Force full range (fixes issue with DATA_FORMAT register's reset value)
  // Per datasheet, needs to be D4=0, D3=D1=D0=1
  Wire.beginTransmission(0x53);
  Wire.write(0x31);
  Wire.write(0b00001011);
  Wire.endTransmission();

  // Sets the data rate for the ADXL343(controls power consumption)
  Wire.beginTransmission(0x53);
  Wire.write(0x2C);
  Wire.write(0b1110);
  Wire.endTransmission();
}


void ADXL375::getAcceleration(float* x, float* y, float* z) {
  Wire.beginTransmission(0x53);
  Wire.write(0x32);
  Wire.endTransmission();
  Wire.requestFrom(0x53, 6);

  int16_t xRaw = ((int16_t)Wire.read() << 8) | (int16_t)Wire.read();
  *x = (float)xRaw * 0.049;

  int16_t yRaw = ((int16_t)Wire.read() << 8) | (int16_t)Wire.read();
  *y = (float)yRaw * 0.049;

  int16_t zRaw = ((int16_t)Wire.read() << 8) | (int16_t)Wire.read();
  *z = (float)zRaw * 0.049;
}