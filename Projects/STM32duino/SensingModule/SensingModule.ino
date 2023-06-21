#include <TaskManager.h>
#include "CANSTM.hpp"
#include "BNO055.hpp"
#include "BME280.hpp"
#include "Thermistor.hpp"
#include "PullupPin.hpp"
#include "OutputPin.hpp"
#include "FRAM.hpp"
#include "Sd.hpp"


namespace flightMode {
  enum class Mode : uint8_t {
    SLEEP,
    STANDBY,
    THRUST,
    CLIMB,
    DESCENT,
    DECEL,
    PARACHUTE,
    LAND,
    SHUTDOWN
  };
}

namespace timer {
  void task10Hz();
  void task20Hz();
  void task100Hz();

  void invalidSdBlink();
}

namespace sensor {
  BNO055 bno;
  BME bme;
  Thermistor thermistor(A1);
}

namespace recorder {
  FRAM fram0(A2);
  FRAM fram1(A3);
  Sd sd(A0);

  PullupPin cardDetection(D8);

  bool doRecording;
}

namespace interface {
  OutputPin canSend(D12);
  OutputPin canReceive(D11);
  OutputPin sdStatus(D9);
  OutputPin recorderStatus(D6);

  OutputPin recorderPower(D7);

  CANSTM can;
}

namespace data {
  float referencePressure = 1013.25;

  float pressure;
  float temperature;
  float altitude;

  float acceleration_x, acceleration_y, acceleration_z;
  float magnetometer_x, magnetometer_y, magnetometer_z;
  float gyroscope_x, gyroscope_y, gyroscope_z;
  float orientation_x, orientation_y, orientation_z;
  float linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
  float gravity_x, gravity_y, gravity_z;

  uint8_t mode;
  bool camera, sn3, sn4;
}

namespace develop {
  PullupPin debugMode(D3);

  bool isDebugMode;
}


void setup() {
  // 起動モードの判定
  develop::isDebugMode = !develop::debugMode.isOpen();


  // デバッグ用シリアルポートの準備
  if (develop::isDebugMode) {
    Serial.begin(115200);
  }


  // 開発中: 保存は常に行う表示
  interface::recorderPower.on();
  interface::recorderStatus.on();

  SPI.setMOSI(A6);
  SPI.setMISO(A5);
  SPI.setSCLK(A4);
  SPI.begin();

  if (recorder::sd.begin()) {
    interface::sdStatus.on();
  }
  else {
    Tasks.add("invalidSdBlink", timer::invalidSdBlink)->startFps(2);
  }

  Wire.setSDA(D4);
  Wire.setSCL(D5);
  Wire.begin();
  Wire.setClock(400000);

  sensor::bno.begin();
  sensor::bme.begin();
  sensor::thermistor.initialize();

  interface::can.begin(500000);

  Tasks.add(timer::task10Hz)->startFps(10);
  Tasks.add(timer::task20Hz)->startFps(20);
  Tasks.add(timer::task100Hz)->startFps(100);

  // メモリ切断処理
  // control::recorderPower.off();
  // SPI.end();
  // pinMode(A6, OUTPUT); // MOSI
  // digitalWrite(A6, LOW);
  // pinMode(A5, OUTPUT); // MISO
  // digitalWrite(A5, LOW);
  // digitalWrite(A2, LOW); // CS FRAM0
  // digitalWrite(A3, LOW); // CS FRAM1
  // digitalWrite(A0, LOW); // CS SD
}


void loop() {
  Tasks.update();

  // SDの検知の更新
  // SDを新しく検知した時
  if (!recorder::doRecording && !recorder::sd.isRunning() && !recorder::cardDetection.isOpen()) {
    recorder::sd.begin();
    Tasks.erase("invalidSdBlink");
    interface::sdStatus.on();
  }

  // SDが検知できなくなった時
  if (!recorder::doRecording && recorder::sd.isRunning() && recorder::cardDetection.isOpen()) {
    recorder::sd.end();
    Tasks.add("invalidSdBlink", timer::invalidSdBlink)->startFps(2);
  }

  if (interface::can.available()) {
    switch (interface::can.getLatestMessageLabel()) {
    case CANSTM::Label::STATUS:
      interface::can.receiveStatus(&data::mode, &data::camera, &data::sn3, &data::sn4);
      break;
    }

    interface::canReceive.toggle();
  }
}


void timer::task10Hz() {
  sensor::thermistor.getTemperature(&data::temperature);
}


void timer::task20Hz() {
  sensor::bno.getMagnetometer(&data::magnetometer_x, &data::magnetometer_y, &data::magnetometer_z);

  interface::can.sendVector3D(CANSTM::Label::ORIENTATION, data::magnetometer_x, data::magnetometer_y, data::magnetometer_z);
  interface::can.sendVector3D(CANSTM::Label::LINEAR_ACCELERATION, data::linear_acceleration_x, data::linear_acceleration_y, data::linear_acceleration_z);
  interface::canSend.toggle();
}


void timer::task100Hz() {
  sensor::bno.getAcceleration(&data::acceleration_x, &data::acceleration_y, &data::acceleration_z);
  sensor::bno.getGyroscope(&data::gyroscope_x, &data::gyroscope_y, &data::gyroscope_z);
  sensor::bno.getOrientation(&data::orientation_x, &data::orientation_y, &data::orientation_z);
  sensor::bno.getLinearAcceleration(&data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
  sensor::bno.getGravityVector(&data::gravity_x, &data::gravity_y, &data::gravity_z);

  sensor::bme.getPressure(&data::pressure);
  data::altitude = (((pow((data::referencePressure / data::pressure), (1.0 / 5.257))) - 1.0) * (data::temperature + 273.15)) / 0.0065;

  interface::can.sendScalar(CANSTM::Label::ALTITUDE, data::altitude);
  interface::canSend.toggle();

  Serial.println(data::mode);
}


void timer::invalidSdBlink() {
  interface::sdStatus.toggle();
}