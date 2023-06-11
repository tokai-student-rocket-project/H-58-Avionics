#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>


namespace internal {
  uint32_t referenceTime;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(500E3);

  pinMode(LED_BUILTIN, OUTPUT);

  MsgPacketizer::subscribe(LoRa, 0x00,
    [](
      uint32_t count,
      float data0,
      float data1,
      float data2,
      float data3,
      float data4,
      float data5,
      float data6,
      float data7,
      float data8,
      float data9,
      float data10,
      float data11
      )
    {
      Serial.print("Received:    RSSI ");

      Serial.print(LoRa.packetRssi());
      Serial.print(" dBm    Routine ");

      uint32_t time = millis();
      Serial.print(1000.0 / (float)(time - internal::referenceTime), 1);
      Serial.print(" Hz    Count ");
      internal::referenceTime = time;

      Serial.println(count);

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  );
}


void loop() {
  if (LoRa.parsePacket()) {
    MsgPacketizer::parse();
  }
}