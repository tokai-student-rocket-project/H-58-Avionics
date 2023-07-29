#include <MsgPacketizer.h>
#include "FRAM.hpp"
#include "LED.hpp"


LED recorderPower(D7);
FRAM fram0(A2);
FRAM fram1(A3);
void dumpRaw();


void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(800);

  recorderPower.on();

  SPI.setMOSI(A6);
  SPI.setMISO(A5);
  SPI.setSCLK(A4);
  SPI.begin();


  MsgPacketizer::subscribe_manual(0xAA,
    [&](uint32_t millis,
      float outsideTemperature, float pressure, float altitude, float climbIndex, bool isFalling,
      float acceleration_x, float acceleration_y, float acceleration_z,
      float gyroscope_x, float gyroscope_y, float gyroscope_z,
      float magnetometer_x, float magnetometer_y, float magnetometer_z,
      float orientation_x, float orientation_y, float orientation_z,
      float linear_acceleration_x, float linear_acceleration_y, float linear_acceleration_z,
      float gravity_x, float gravity_y, float gravity_z) {
        Serial.print(millis); Serial.print(",");
        Serial.print(outsideTemperature); Serial.print(",");
        Serial.print(pressure); Serial.print(",");
        Serial.print(altitude); Serial.print(",");
        Serial.print(climbIndex); Serial.print(",");
        Serial.print(isFalling); Serial.print(",");
        Serial.print(acceleration_x); Serial.print(",");
        Serial.print(acceleration_y); Serial.print(",");
        Serial.print(acceleration_z); Serial.print(",");
        Serial.print(gyroscope_x); Serial.print(",");
        Serial.print(gyroscope_y); Serial.print(",");
        Serial.print(gyroscope_z); Serial.print(",");
        Serial.print(magnetometer_x); Serial.print(",");
        Serial.print(magnetometer_y); Serial.print(",");
        Serial.print(magnetometer_z); Serial.print(",");
        Serial.print(orientation_x); Serial.print(",");
        Serial.print(orientation_y); Serial.print(",");
        Serial.print(orientation_z); Serial.print(",");
        Serial.print(linear_acceleration_x); Serial.print(",");
        Serial.print(linear_acceleration_y); Serial.print(",");
        Serial.print(linear_acceleration_z); Serial.print(",");
        Serial.print(gravity_x); Serial.print(",");
        Serial.print(gravity_y); Serial.print(",");
        Serial.print(gravity_z); Serial.println();
    }
  );

  dumpRaw(&fram0);
  dumpRaw(&fram1);
}


void loop() {
}


void dumpRaw(FRAM* fram) {
  uint8_t data[256];
  uint32_t size = 0;
  uint32_t writeAddress = 0;

  for (uint32_t address = 0; address < FRAM::LENGTH; address++) {
    data[writeAddress] = fram->read(address);
    size = writeAddress + 1;

    if (data[writeAddress] == 0x00) {
      writeAddress = 0;
      MsgPacketizer::feed(data, size);
    }
    else {
      writeAddress++;
    }
  }
}