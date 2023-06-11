#include <SPI.h>
#include <LoRa.h>
#include <TaskManager.h>


namespace settings {
  // 設定する値たち

  // 周波数
  // 初期値: 921.8E6 (921.8 MHz)
  int32_t frequency = 921.8E6;

  // 帯域幅
  // 初期値: 125E3 (125 kHz)
  int32_t bandwidth = 500E3;

  // 送信回数
  // 初期値: 100 (100 Hz)
  int16_t taskFrequency = 100;

  // 送信データ長
  // 初期値: 32 (32 Bytes)
  size_t length = 64;
}


namespace internal {
  uint32_t referenceTime;

  uint16_t count = 0;
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

  Tasks.add(routine)->startIntervalMsec(1000 / settings::taskFrequency);

  pinMode(LED_BUILTIN, OUTPUT);
}


void loop() {
  Tasks.update();
}


void routine() {
  uint8_t data[settings::length];

  internal::count++;
  internal::converter.value = internal::count;

  data[0] = internal::converter.data[0];
  data[1] = internal::converter.data[1];
  data[2] = internal::converter.data[2];
  data[3] = internal::converter.data[3];

  LoRa.beginPacket(true);
  LoRa.write(data, settings::length);
  LoRa.endPacket();

  Serial.print("Sended:    Routine ");

  uint32_t time = millis();
  Serial.print(1000.0 / (float)(time - internal::referenceTime), 1);
  Serial.print(" Hz    count ");
  internal::referenceTime = time;

  Serial.println(internal::count);

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}