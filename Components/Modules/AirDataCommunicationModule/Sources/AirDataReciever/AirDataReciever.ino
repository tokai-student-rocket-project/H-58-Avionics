#include <SPI.h>
#include <LoRa.h>
#include <MsgPacketizer.h>
#include <ArduinoJson.h>
#include <movingAvg.h>
#include <Calculus.h>


namespace transmitter {
  StaticJsonDocument<1024> packet;
}

namespace smooth {
  movingAvg altitudeSmooth(10);
  movingAvg climbRateSmooth(10);
  movingAvg outsideTemperatureSmooth(10);
  movingAvg internalTemperatureSmooth(10);
  movingAvg coldTemperatureSmooth(10);
  movingAvg linearAccelerationXSmooth(10);
  movingAvg linearAccelerationYSmooth(10);
  movingAvg linearAccelerationZSmooth(10);
  movingAvg orientationXSmooth(10);
  movingAvg orientationYSmooth(10);
  movingAvg orientationZSmooth(10);

  Calculus::Differential<float> climbRateDifferential(1.0);
  float t_prev;
}


void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  // ① 34ch 922.6E6
  // ② 30ch 921.8E6
  LoRa.begin(922.6E6);
  LoRa.setSignalBandwidth(500E3);

  smooth::altitudeSmooth.begin();
  smooth::climbRateSmooth.begin();
  smooth::outsideTemperatureSmooth.begin();
  smooth::internalTemperatureSmooth.begin();
  smooth::coldTemperatureSmooth.begin();
  smooth::linearAccelerationXSmooth.begin();
  smooth::linearAccelerationYSmooth.begin();
  smooth::linearAccelerationZSmooth.begin();
  smooth::orientationXSmooth.begin();
  smooth::orientationYSmooth.begin();
  smooth::orientationZSmooth.begin();

  MsgPacketizer::subscribe(LoRa, 0x00,
    [](
      uint32_t ms,
      float altitude,
      float climbRate,
      float outsideTemperature,
      float internalTemperature,
      float coldTemperature,
      float orientation_x,
      float orientation_y,
      float orientation_z,
      float linear_acceleration_x,
      float linear_acceleration_y,
      float linear_acceleration_z
      )
    {
      float t = millis() / 1000.0;
      float dt = t - smooth::t_prev;
      smooth::t_prev = t;
      float climbRateFromDifferential = smooth::climbRateDifferential.get(altitude, dt);

      transmitter::packet.clear();
      transmitter::packet["PacketInfo"]["Sender"] = "ACM";
      transmitter::packet["PacketInfo"]["Type"] = "AirData";
      transmitter::packet["PacketInfo"]["RSSI"] = LoRa.packetRssi();
      transmitter::packet["PacketInfo"]["SNR"] = LoRa.packetSnr();
      transmitter::packet["PacketInfo"]["Millis"] = ms;
      transmitter::packet["Alt"] = (float)smooth::altitudeSmooth.reading((int16_t)(altitude * 100)) / 100.0;
      transmitter::packet["CR"] = (float)smooth::climbRateSmooth.reading((int16_t)(climbRateFromDifferential * 100)) / 100.0;
      transmitter::packet["OutTemp"] = (float)smooth::outsideTemperatureSmooth.reading((int16_t)(outsideTemperature * 100)) / 100.0;
      transmitter::packet["inTemp"] = (float)smooth::internalTemperatureSmooth.reading((int16_t)(internalTemperature * 100)) / 100.0;
      transmitter::packet["CldTemp"] = (float)smooth::coldTemperatureSmooth.reading((int16_t)(coldTemperature * 100)) / 100.0;
      transmitter::packet["Ori"]["x"] = (float)smooth::orientationXSmooth.reading((int16_t)(orientation_x * 100)) / 100.0;
      transmitter::packet["Ori"]["y"] = (float)smooth::orientationYSmooth.reading((int16_t)(orientation_y * 100)) / 100.0;
      transmitter::packet["Ori"]["z"] = (float)smooth::orientationZSmooth.reading((int16_t)(orientation_z * 100)) / 100.0;
      transmitter::packet["Lia"]["x"] = (float)smooth::linearAccelerationXSmooth.reading((int16_t)(linear_acceleration_x * 100)) / 100.0;
      transmitter::packet["Lia"]["y"] = (float)smooth::linearAccelerationYSmooth.reading((int16_t)(linear_acceleration_y * 100)) / 100.0;
      transmitter::packet["Lia"]["z"] = (float)smooth::linearAccelerationZSmooth.reading((int16_t)(linear_acceleration_z * 100)) / 100.0;

      serializeJson(transmitter::packet, Serial);
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