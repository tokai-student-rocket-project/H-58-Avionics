#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>


void setup() {
  // ログ出力用のシリアルポートをセットアップ
  Serial.begin(115200);
  delay(800);

  // LoRaのセットアップ
  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(250E3);
  pinMode(LED_BUILTIN, OUTPUT);

  // 受信したパケットをパースする処理を登録
  // 登録するだけなので必要な時に呼び出される
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
  // ダウンリンクを受信していればパースする
  if (LoRa.parsePacket()) {
    MsgPacketizer::parse();
  }
}