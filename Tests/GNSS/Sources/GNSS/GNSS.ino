#include <SPI.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <TaskManager.h>
#include "OutputPin.hpp"
#include "GNSS.hpp"


namespace timer {
  void task10Hz();
}

namespace sensor {
  GNSS gnss;
}

namespace indicator {
  OutputPin gpsStatus(5);
}

namespace data {
  float latitude;
  float longitude;

  StaticJsonDocument<256> packet;
}


void setup() {
  Serial.begin(115200);

  sensor::gnss.begin();

  Tasks.add(timer::task10Hz)->startFps(10);
}


void loop() {
  Tasks.update();
}


void timer::task10Hz() {
  if (sensor::gnss.available()) {
    data::latitude = sensor::gnss.getLatitude();
    data::longitude = sensor::gnss.getLongitude();

    indicator::gpsStatus.toggle();
  }

  data::packet["lat"] = data::latitude;
  data::packet["lon"] = data::longitude;

  serializeJson(data::packet, Serial);
  Serial.println();
  data::packet.clear();
}