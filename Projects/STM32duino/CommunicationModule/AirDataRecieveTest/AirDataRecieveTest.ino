#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <ArduinoJson.h>


namespace transmitter {
  StaticJsonDocument<1024> packet;
}


void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.begin(923.8E6);

  MsgPacketizer::subscribe(LoRa, 0x00,
    [](
      float altitude,
      float acceleration_x,
      float acceleration_y,
      float acceleration_z,
      float magnetometer_x,
      float magnetometer_y,
      float magnetometer_z,
      float gyroscope_x,
      float gyroscope_y,
      float gyroscope_z,
      float orientation_x,
      float orientation_y,
      float orientation_z,
      float linear_acceleration_x,
      float linear_acceleration_y,
      float linear_acceleration_z
      )
    {
      transmitter::packet["alt"] = altitude;
      transmitter::packet["acc"]["x"] = acceleration_x;
      transmitter::packet["acc"]["y"] = acceleration_y;
      transmitter::packet["acc"]["z"] = acceleration_z;
      transmitter::packet["mag"]["x"] = magnetometer_x;
      transmitter::packet["mag"]["y"] = magnetometer_y;
      transmitter::packet["mag"]["z"] = magnetometer_z;
      transmitter::packet["gyr"]["x"] = gyroscope_x;
      transmitter::packet["gyr"]["y"] = gyroscope_y;
      transmitter::packet["gyr"]["z"] = gyroscope_z;
      transmitter::packet["ori"]["x"] = orientation_x;
      transmitter::packet["ori"]["y"] = orientation_y;
      transmitter::packet["ori"]["z"] = orientation_z;
      transmitter::packet["lia"]["x"] = linear_acceleration_x;
      transmitter::packet["lia"]["y"] = linear_acceleration_y;
      transmitter::packet["lia"]["z"] = linear_acceleration_z;


      serializeJson(transmitter::packet, Serial);
      Serial.println();
      transmitter::packet.clear();

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  );
}


void loop() {
  if (LoRa.parsePacket()) {
    MsgPacketizer::parse();
  }
}