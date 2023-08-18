#include <MsgPacketizer.h>
#include "ADXL375.hpp"
#include "FRAM.hpp"
#include "LED.hpp"


// SPIは使わなくてもCSをHIGHにする必要があるよ
ADXL375 adxl(15);

LED recorderPower(5);
FRAM fram0(A6);
FRAM fram1(A5);

void dump(FRAM* fram);


void setup() {
  Serial.begin(115200);

  recorderPower.on();

  SPI.begin();

  MsgPacketizer::subscribe_manual(0xAA,
    [&](uint32_t micros, uint8_t flightMode,
      uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1, uint8_t z0, uint8_t z1
      ) {
        Serial.print(micros); Serial.print(",");
        Serial.print(flightMode); Serial.print(",");
        Serial.print(x0); Serial.print(",");
        Serial.print(x1); Serial.print(",");
        Serial.print(y0); Serial.print(",");
        Serial.print(y1); Serial.print(",");
        Serial.print(z0); Serial.print(",");
        Serial.print(z1); Serial.println();
    }
  );

  while (!Serial);
  delay(800);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  dump(&fram0);
  dump(&fram1);
  digitalWrite(LED_BUILTIN, LOW);
}


void loop() {
}


void dump(FRAM* fram) {
  uint8_t data[4096];
  uint32_t size = 0;
  uint32_t writeAddress = 0;

  for (uint32_t address = 0; address < FRAM::LENGTH; address++) {
    data[writeAddress] = fram->read(address);
    size = writeAddress + 1;

    if (data[writeAddress] == 0x00) {
      writeAddress = 0;

      if (data[1] == 0xAA) {
        MsgPacketizer::feed(data, size);
      }
    }
    else {
      writeAddress++;
    }
  }
}