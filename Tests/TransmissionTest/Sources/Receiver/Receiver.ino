#include <SPI.h>
#include <LoRa.h>


namespace settings {
  // 設定する値たち

  // 周波数
  // 初期値: 921.8E6 (921.8 MHz)
  int32_t frequency = 921.8E6;

  // 帯域幅
  // 初期値: 125E3 (125 kHz)
  int32_t bandwidth = 500E3;
}

namespace internal {
  uint32_t referenceTime;

  union Converter {
    uint16_t value;
    uint8_t data[4];
  }converter;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(settings::frequency);
  LoRa.setSpreadingFactor(6);
  LoRa.setSignalBandwidth(settings::bandwidth);
  LoRa.setPreambleLength(6);

  pinMode(LED_BUILTIN, OUTPUT);
}


void loop() {
  int16_t packetSize = LoRa.parsePacket(64);

  if (packetSize) {
    for (size_t i = 0; i < 64; i++) {

      if (i < 4) {
        internal::converter.data[i] = LoRa.read();
      }
      else {
        LoRa.read();
      }
    }

    Serial.print("Received:    RSSI ");

    Serial.print(LoRa.packetRssi());
    Serial.print(" dBm    Routine ");

    uint32_t time = millis();
    Serial.print(1000.0 / (float)(time - internal::referenceTime), 1);
    Serial.print(" Hz    Count ");
    internal::referenceTime = time;

    Serial.println(internal::converter.value);

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}