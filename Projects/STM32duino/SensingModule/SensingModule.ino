#include <TaskManager.h>
#include "CANSTM.hpp"
#include "BNO055.hpp"
#include "BME280.hpp"
#include "Thermistor.hpp"
#include "PullupPin.hpp"
#include "OutputPin.hpp"
#include "Trajectory.hpp"
#include "Blinker.hpp"
#include "Recorder.hpp"
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
}

namespace sensor {
  BNO055 bno;
  BME bme;
  Thermistor thermistor(A1);
}

namespace recorder {
  Recorder recorder(A2, A3);
  Sd sd(A0);
  PullupPin cardDetection(D8);

  bool doRecording;
}

namespace indicator {
  OutputPin canSend(D12);
  OutputPin canReceive(D11);
  Blinker sdStatus(D9, "invalidSd");
  OutputPin recorderStatus(D6);
}

namespace control {
  OutputPin recorderPower(D7);
}

namespace connection {
  CANSTM can;
}

namespace data {
  Trajectory trajectory(0.25, 0.75);

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
  bool camera, separator;
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
    while (!Serial);
    delay(800);
  }

  // 開発中: 保存は常に行う表示
  control::recorderPower.on();
  indicator::recorderStatus.on();

  SPI.setMOSI(A6);
  SPI.setMISO(A5);
  SPI.setSCLK(A4);
  SPI.begin();

  if (recorder::sd.begin()) {
    indicator::sdStatus.on();
  }
  else {
    indicator::sdStatus.startBlink(2);
  }

  Wire.setSDA(D4);
  Wire.setSCL(D5);
  Wire.begin();
  Wire.setClock(400000);

  sensor::bno.begin();
  sensor::bme.begin();
  sensor::thermistor.initialize();

  connection::can.begin();

  connection::can.sendEvent(CANSTM::Publisher::SENSING_MODULE, CANSTM::EventCode::SETUP);

  Tasks.add(timer::task20Hz)->startFps(20);
  Tasks.add(timer::task100Hz)->startFps(100);
}


void loop() {
  Tasks.update();

  // SDの検知の更新
  // SDを新しく検知した時
  if (!recorder::doRecording && !recorder::sd.isRunning() && !recorder::cardDetection.isOpen()) {
    recorder::sd.begin();
    indicator::sdStatus.stopBlink();
    indicator::sdStatus.on();
  }

  // SDが検知できなくなった時
  if (!recorder::doRecording && recorder::sd.isRunning() && recorder::cardDetection.isOpen()) {
    recorder::sd.end();
    indicator::sdStatus.startBlink(2);
  }

  //CAN受信処理
  if (connection::can.available()) {
    switch (connection::can.getLatestMessageLabel()) {
    case CANSTM::Label::SYSTEM_STATUS:
      connection::can.receiveStatus(&data::mode, &data::camera, &data::separator);
      indicator::canReceive.toggle();

      // フライトモードがSTANDBYとLANDの間ならログを保存する
      if (1 <= data::mode && data::mode <= 7) {
        if (!recorder::doRecording) {
          recorder::doRecording = true;
          recorder::recorder.reset();
          indicator::recorderStatus.on();
        }
      }
      else {
        if (recorder::doRecording) {
          recorder::doRecording = false;
          indicator::recorderStatus.off();
        }
      }

      break;
    case CANSTM::Label::SET_REFERENCE_PRESSURE_COMMAND:
      float newReferencePressure;
      connection::can.receiveSetReferencePressureCommand(&newReferencePressure);
      data::trajectory.setReferencePressure(newReferencePressure);
      indicator::canReceive.toggle();
      connection::can.sendEvent(CANSTM::Publisher::SENSING_MODULE, CANSTM::EventCode::REFERENCE_PRESSURE_UPDATED);
      indicator::canSend.toggle();
      break;
    }
  }
}


void timer::task20Hz() {
  sensor::bno.getMagnetometer(&data::magnetometer_x, &data::magnetometer_y, &data::magnetometer_z);
  sensor::thermistor.getTemperature(&data::temperature);

  data::altitude = data::trajectory.update(data::pressure, data::temperature);

  connection::can.sendScalar(CANSTM::Label::TEMPERATURE, data::temperature);
  connection::can.sendScalar(CANSTM::Label::ALTITUDE, data::altitude);
  connection::can.sendTrajectoryData(data::trajectory.isFalling());
  connection::can.sendVector3D(CANSTM::Label::ORIENTATION, data::magnetometer_x, data::magnetometer_y, data::magnetometer_z);
  connection::can.sendVector3D(CANSTM::Label::LINEAR_ACCELERATION, data::linear_acceleration_x, data::linear_acceleration_y, data::linear_acceleration_z);
  indicator::canSend.toggle();
}


void timer::task100Hz() {
  sensor::bno.getAcceleration(&data::acceleration_x, &data::acceleration_y, &data::acceleration_z);
  sensor::bno.getGyroscope(&data::gyroscope_x, &data::gyroscope_y, &data::gyroscope_z);
  sensor::bno.getOrientation(&data::orientation_x, &data::orientation_y, &data::orientation_z);
  sensor::bno.getLinearAcceleration(&data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
  sensor::bno.getGravityVector(&data::gravity_x, &data::gravity_y, &data::gravity_z);
  sensor::bme.getPressure(&data::pressure);


  if (recorder::doRecording) {
    recorder::recorder.record(
      millis(),
      data::temperature, data::pressure, data::altitude, data::trajectory.climbIndex(), data::trajectory.isFalling(),
      data::acceleration_x, data::acceleration_y, data::acceleration_z,
      data::gyroscope_x, data::gyroscope_y, data::gyroscope_z,
      data::magnetometer_x, data::magnetometer_y, data::magnetometer_z,
      data::orientation_x, data::orientation_y, data::orientation_z,
      data::linear_acceleration_x, data::linear_acceleration_y, data::linear_acceleration_z,
      data::gravity_x, data::gravity_y, data::gravity_z
    );
  }
}