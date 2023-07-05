#include "ADXL375.hpp"


ADXL375::ADXL375(uint32_t cs) {
  _cs = cs;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
}


void ADXL375::begin() {
  // Disable interrupts to start
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b01000000 | 0x2E); // Address: Write | INT_ENABLE
  SPI.transfer(0b00000000); // Data: Reset
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  // 62.5 mg/LSB (so 0xFF = 16 g)
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b01000000 | 0x1D); // Address: Write | THRESH_SHOCK
  SPI.transfer(20);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  // Max tap duration, 625 µs/LSB
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b01000000 | 0x21); // Address: Write | DUR
  SPI.transfer(50);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  // Tap latency, 1.25 ms/LSB, 0=no double tap
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b01000000 | 0x22); // Address: Write | Latent
  SPI.transfer(0b00000000); // Data: Reset
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  // Waiting period,  1.25 ms/LSB, 0=no double tap
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b01000000 | 0x23); // Address: Write | Window
  SPI.transfer(0b00000000); // Data: Reset
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  // Enable the XYZ axis for tap
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b01000000 | 0x2A); // Address: Write | SHOCK_AXES
  SPI.transfer(0b00000111); // Data: SHOCK_X,Y,Z enable
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  // Enable measurements
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b01000000 | 0x2D); // Address: Write | POWER_CTL
  SPI.transfer(0x00001000); // Data: Measure
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  // Force full range (fixes issue with DATA_FORMAT register's reset value)
  // Per datasheet, needs to be D4=0, D3=D1=D0=1
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b01000000 | 0x31); // Address: Write | DATA_FORMAT
  SPI.transfer(0b00001011);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  // Sets the data rate for the ADXL343(controls power consumption)
  SPI.beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  SPI.transfer(0b01000000 | 0x2C); // Address: Write | BW_RATE
  SPI.transfer(0b1110);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
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