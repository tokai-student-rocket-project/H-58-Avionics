#include "ADXL375.hpp"


ADXL375::ADXL375(uint32_t cs) {
  _cs = cs;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
}


void ADXL375::begin() {
  // INT_ENABLE <- Reset
  write(0x2E, 0b00000000);

  // THRESH_SHOCK
  write(0x1D, 20);

  // DUR
  write(0x21, 50);

  // Tap latency, 1.25 ms/LSB, 0=no double tap
  // Latent <- Reset
  write(0x22, 0b00000000);

  // Waiting period,  1.25 ms/LSB, 0=no double tap
  // Window <- Reset
  write(0x23, 0b00000000);

  // Enable the XYZ axis for tap
  // SHOCK_AXES <- SHOCK_X,Y,Z enable
  write(0x2A, 0b00000111);

  // Enable measurements
  // POWER_CTL <- Measure
  write(0x2D, 0x00001000);

  // Force full range (fixes issue with DATA_FORMAT register's reset value)
  // Per datasheet, needs to be D4=0, D3=D1=D0=1
  // DATA_FORMAT
  write(0x31, 0b00001011);

  // Sets the data rate for the ADXL343(controls power consumption)
  // BW_RATE
  write(0x2C, 0b1110);
}


void ADXL375::getAcceleration(float* x, float* y, float* z) {
  // Read x
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b11000000 | 0x32); // Address: Read | DATAX0
  uint8_t xRaw0 = SPI.transfer(0x00);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b11000000 | 0x33); // Address: Read | DATAX1
  uint8_t xRaw1 = SPI.transfer(0x00);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  // Read y
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b11000000 | 0x34); // Address: Read | DATAY0
  uint8_t yRaw0 = SPI.transfer(0x00);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b11000000 | 0x35); // Address: Read | DATAY1
  uint8_t yRaw1 = SPI.transfer(0x00);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  // Read z
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b11000000 | 0x36); // Address: Read | DATAZ0
  uint8_t zRaw0 = SPI.transfer(0x00);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b11000000 | 0x37); // Address: Read | DATAZ1
  uint8_t zRaw1 = SPI.transfer(0x00);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  int16_t xRaw = ((int16_t)xRaw0 << 8) | (int16_t)xRaw1;
  *x = (float)xRaw * 0.049;

  int16_t yRaw = ((int16_t)yRaw0 << 8) | (int16_t)yRaw1;
  *y = (float)yRaw * 0.049;

  int16_t zRaw = ((int16_t)zRaw0 << 8) | (int16_t)zRaw1;
  *z = (float)zRaw * 0.049;
}


void ADXL375::write(uint8_t address, uint8_t data) {
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b01000000 | address);
  SPI.transfer(data);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}