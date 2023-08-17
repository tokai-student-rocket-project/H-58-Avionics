#include <MsgPacketizer.h>
#include "FRAM.hpp"
#include "LED.hpp"


LED recorderPower(5);
FRAM fram0(A6);
FRAM fram1(A5);
void dump(FRAM* fram);


void setup() {
  Serial.begin(115200);

  recorderPower.on();

  SPI.begin();

  MsgPacketizer::subscribe_manual(0xAA,
    [&](uint32_t millis, uint8_t flightMode,
      float x, float y, float z
      ) {
        Serial.print(millis); Serial.print(",");
        Serial.print(flightMode); Serial.print(",");
        Serial.print(x); Serial.print(",");
        Serial.print(y); Serial.print(",");
        Serial.print(z); Serial.println();
    }
  );

  while (!Serial);
  delay(5000);
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

    Serial.println(data[writeAddress], HEX);

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