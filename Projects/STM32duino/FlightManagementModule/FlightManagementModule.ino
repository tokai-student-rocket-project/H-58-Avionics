#include <ACAN_STM32.h>


void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  ACAN_STM32_Settings settings(1000000); // 1 Mbit/s
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);
}


void loop() {
  CANMessage message;
  if (can.receive0(message)) {
    uint32_t valueRaw = (uint32_t)(
      (((uint32_t)message.data[0] << 24) & 0xFF000000)
      | (((uint32_t)message.data[1] << 16) & 0x00FF0000)
      | (((uint32_t)message.data[2] << 8) & 0x0000FF00)
      | (((uint32_t)message.data[3] << 0) & 0x000000FF)
      );

    Serial.print("Received: ");
    Serial.println((double)valueRaw / 4096.0);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}