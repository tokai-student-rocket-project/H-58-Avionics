#include <SPI.h>
#include <LoRa.h>


namespace settings {
  // 設定する値たち

  // 周波数
  // 初期値: 921.8E6 (921.8 MHz)
  int32_t frequency = 921.8E6;

  // 帯域幅
  // 初期値: 125E3 (125 kHz)
  int32_t bandwidth = 125E3;
}

namespace internal {
  uint32_t referenceTime;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(settings::frequency);
  LoRa.setSignalBandwidth(settings::bandwidth);

  pinMode(LED_BUILTIN, OUTPUT);
}


void loop() {
  int16_t packetSize = LoRa.parsePacket();

  if (packetSize) {
    while (LoRa.available()) {
      LoRa.read();
    }

    Serial.print("Received:    Frequency ");
    Serial.print((float)settings::frequency / 1000000.0, 1);
    Serial.print(" MHz    Bandwidth ");
    Serial.print((float)settings::bandwidth / 1000.0, 0);
    Serial.print(" kHz    Length ");
    Serial.print(packetSize);
    Serial.print(" Bytes    RSSI ");

    Serial.print(LoRa.packetRssi());
    Serial.print(" dBm    Routine ");

    uint32_t time = millis();
    Serial.print(1000.0 / (float)(time - internal::referenceTime), 1);
    Serial.println(" Hz");
    internal::referenceTime = time;

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}