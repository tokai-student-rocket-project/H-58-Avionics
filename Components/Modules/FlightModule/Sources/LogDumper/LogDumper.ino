#include <MsgPacketizer.h>
#include "FRAM.hpp"


FRAM fram(D4);
void dump(FRAM* fram);


void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(800);

  SPI.setMOSI(A6);
  SPI.setMISO(A5);
  SPI.setSCLK(A4);
  SPI.begin();


  MsgPacketizer::subscribe_manual(0xAA,
    [&](uint32_t millis, uint32_t flightTime,
      uint8_t flightMode, bool cameraState, bool sn3State, bool doLogging,
      bool isFalling, bool flightPinState, bool resetPinState,
      float supplyVoltage, float batteryVoltage, float poolVoltage) {
        Serial.print(millis); Serial.print(",");
        Serial.print(flightTime); Serial.print(",");
        Serial.print(flightMode); Serial.print(",");
        Serial.print(cameraState); Serial.print(",");
        Serial.print(sn3State); Serial.print(",");
        Serial.print(doLogging); Serial.print(",");
        Serial.print(isFalling); Serial.print(",");
        Serial.print(flightPinState); Serial.print(",");
        Serial.print(resetPinState); Serial.print(",");
        Serial.print(supplyVoltage); Serial.print(",");
        Serial.print(batteryVoltage); Serial.print(",");
        Serial.print(poolVoltage); Serial.println();
    }
  );

  dump(&fram);
}


void loop() {
}


void dump(FRAM* fram) {
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