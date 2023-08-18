#include <MsgPacketizer.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(800);

  uint32_t time = micros();
  uint8_t flightMode = 9;

  uint8_t x0 = 0xAA;
  uint8_t x1 = 0xAA;
  uint8_t y0 = 0xAA;
  uint8_t y1 = 0xAA;
  uint8_t z0 = 0xAA;
  uint8_t z1 = 0xAA;

  const auto& packet = MsgPacketizer::encode(
    0xAA, time, flightMode,
    x0, x1, y0, y1, z0, z1
  );

  const uint8_t* packetData = packet.data.data();
  const uint32_t packetSize = packet.data.size();

  uint8_t data[24] = { 0 };
  uint32_t size = 24;

  memcpy(data, packetData, packetSize);

  for (size_t i = 0; i < size; i++) {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }

  Serial.println();
}

void loop() {
}