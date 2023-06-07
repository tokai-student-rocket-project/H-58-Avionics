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
  int32_t bandwidth = 125E3;

  // 送信回数
  // 初期値: 10 (10 Hz)
  int16_t taskFrequency = 10;

  // 送信データ長
  // 初期値: 32 (32 Bytes)
  size_t length = 32;
}


namespace internal {
  uint32_t referenceTime;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(settings::frequency);
  LoRa.setSignalBandwidth(settings::bandwidth);

  Tasks.add(routine)->startIntervalMsec(1000 / settings::taskFrequency);

  pinMode(LED_BUILTIN, OUTPUT);
}


void loop() {
  Tasks.update();
}


void routine() {
  uint8_t data[settings::length];

  LoRa.beginPacket();
  LoRa.write(data, settings::length);
  LoRa.endPacket();

  Serial.print("Sended:    Frequency ");
  Serial.print((float)settings::frequency / 1000000.0, 1);
  Serial.print(" MHz    Bandwidth ");
  Serial.print((float)settings::bandwidth / 1000.0, 0);
  Serial.print(" kHz    Length ");
  Serial.print(settings::length);
  Serial.print(" Bytes    Routine ");

  uint32_t time = millis();
  Serial.print(1000.0 / (float)(time - internal::referenceTime), 1);
  Serial.println(" Hz");
  internal::referenceTime = time;

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}