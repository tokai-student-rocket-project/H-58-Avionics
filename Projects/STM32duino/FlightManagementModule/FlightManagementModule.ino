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
    double xRaw = ((int16_t)message.data[0]) | (((int16_t)message.data[1]) << 8);
    double yRaw = ((int16_t)message.data[2]) | (((int16_t)message.data[3]) << 8);
    double zRaw = ((int16_t)message.data[4]) | (((int16_t)message.data[5]) << 8);

    Serial.println(xRaw / 16.0);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}