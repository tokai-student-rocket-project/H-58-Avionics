#include <SPI.h>
#include <LoRa.h>
#include <TaskManager.h>
#include <MsgPacketizer.h>


namespace internal {
  uint32_t referenceTime;
  uint32_t count = 0;

  float data0 = 1.0;
  float data1 = 2.0;
  float data2 = 4.0;

  float data3 = 8.0;
  float data4 = 16.0;
  float data5 = 32.0;

  float data6 = 64.0;
  float data7 = 128.0;
  float data8 = 256.0;

  float data9 = 512.0;
  float data10 = 1024.0;
  float data11 = 2048.0;
}


void setup() {
  Serial.begin(115200);

  LoRa.begin(921.8E6);
  LoRa.setSignalBandwidth(500E3);

  Tasks.add(routine)->startIntervalMsec(10);

  pinMode(LED_BUILTIN, OUTPUT);
}


void loop() {
  Tasks.update();
}


void routine() {
  internal::count++;

  const auto& packet = MsgPacketizer::encode(
    0x00,
    internal::count,
    internal::data0,
    internal::data1,
    internal::data2,
    internal::data3,
    internal::data4,
    internal::data5,
    internal::data6,
    internal::data7,
    internal::data8,
    internal::data9,
    internal::data10,
    internal::data11
  );

  LoRa.beginPacket(true);
  LoRa.write(packet.data.data(), packet.data.size());
  LoRa.endPacket();

  Serial.print("Sended:    Size ");

  Serial.print(packet.data.size());

  Serial.print(" B    Routine");

  uint32_t time = millis();
  Serial.print(1000.0 / (float)(time - internal::referenceTime), 1);
  Serial.print(" Hz    Count ");
  internal::referenceTime = time;

  Serial.println(internal::count);

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}