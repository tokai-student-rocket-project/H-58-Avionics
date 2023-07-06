#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>


void setup() {
  Serial.begin(115200);
  delay(800);

  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(250E3);

  MsgPacketizer::subscribe(LoRa, 0x00,
    [](
      uint32_t millis,
      float internalAmbientTemperature,
      float supplyVoltage,
      float batteryVoltage,
      float poolVoltage
      )
    {
      Serial.print("Log,");
      Serial.print(millis);
      Serial.print(",");
      Serial.print(internalAmbientTemperature);
      Serial.print(",");
      Serial.print(supplyVoltage);
      Serial.print(",");
      Serial.print(batteryVoltage);
      Serial.print(",");
      Serial.print(poolVoltage);
      Serial.println();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  );
}


void loop() {
  if (LoRa.parsePacket()) {
    MsgPacketizer::parse();
  }
}