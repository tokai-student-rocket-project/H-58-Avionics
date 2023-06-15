#include <ACAN_STM32.h>
#include <TaskManager.h>
#include "BNO055.hpp"
#include "BME280.hpp"
#include "Thermistor.hpp"
#include "PullupPin.hpp"
#include "OutputPin.hpp"
#include "FRAM.hpp"
#include "Sd.hpp"


namespace canbus {
  enum class Id : uint32_t {
    TEMPERATURE,
    PRESSURE,
    ALTITUDE,
    ACCELERATION,
    GYROSCOPE,
    MAGNETOMETER,
    ORIENTATION,
    LINEAR_ACCELERATION,
    GRAVITY,
    STATUS
  };

  enum class Axis : uint8_t {
    X,
    Y,
    Z
  };

  union Converter {
    float value;
    uint8_t data[4];
  }converter;

  void initialize();
  void sendScalar(canbus::Id id, float value);
  void sendVector(canbus::Id id, canbus::Axis axis, float value);
  void receiveStatus(CANMessage message);
}

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

namespace indicator {
  OutputPin canSend(D12);
  OutputPin canReceive(D11);

  OutputPin sdStatus(D9);
  OutputPin recorderStatus(D6);
}

namespace control {
  OutputPin recorderPower(D7);
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
    delay(800);
    Serial.println("------ Sensing Module ------");
    Serial.println("Boot Mode: DEBUG");

    Serial.println("");
    Serial.println("            X-- | D1       VIN | <== +12V ");
    Serial.println("            X-- | D0       GND | <== GND");
    Serial.println("            X-- | NRST    NRST | --X");
    Serial.println("        GND ==> | GND      +5V | ==> +5V");
    Serial.println("     CAN_TX <-- | D2        A7 | <-- DUMP_MODE");
    Serial.println(" DEBUG_MODE --> | D3        A6 | --> SPI_MOSI");
    Serial.println("    I2C_SDA <-> | D4        A5 | <-- SPI_MISO");
    Serial.println("    I2C_SCL <-- | D5        A4 | --> SPI_SCLK");
    Serial.println("RECORDER_ST <-- | D6        A3 | --> SPI_CS_FRAM1");
    Serial.println("RECORDER_CT <-- | D7        A2 | --> SPI_CS_FRAM0");
    Serial.println("   SD_CHECK --> | D8        A1 | <-- THERMISTOR");
    Serial.println("      SD_ST --> | D9        A0 | --> SPI_CS_SD");
    Serial.println("     CAN_RX <-- | D10     AREF | --X");
    Serial.println("  CAN_RX_ST <-- | D11     +3V3 | ==> +3.3V");
    Serial.println("  CAN_TX_ST <-- | D12      D13 | --X");
    Serial.println("");
  }


  // 開発中: 保存は常に行う表示
  control::recorderPower.on();
  indicator::recorderStatus.on();


  // SPIの初期化
  if (develop::isDebugMode) {
    Serial.println("--------------------------------------------------------------------------------");
    Serial.println("Initializing SPI...");
  }

  SPI.setMOSI(A6);
  SPI.setMISO(A5);
  SPI.setSCLK(A4);
  SPI.begin();

  if (develop::isDebugMode) {
    Serial.println("SPI: OK");
    Serial.println("Initializing SPI Devices...");
  }


  // SDの初期化
  bool isSucceededBeginSD = recorder::sd.begin();

  if (isSucceededBeginSD) {
    indicator::sdStatus.on();
    if (develop::isDebugMode) Serial.println("    SD: Succeed");
  }
  else {
    Tasks.add("invalidSdBlink", timer::invalidSdBlink)->startIntervalMsec(500);
    if (develop::isDebugMode) Serial.println("    SD: Failed (Not inserted)");
  }


  // I2Cの初期化
  if (develop::isDebugMode) {
    Serial.println("--------------------------------------------------------------------------------");
    Serial.println("Initializing I2C...");
  }

  Wire.setSDA(D4);
  Wire.setSCL(D5);
  Wire.begin();
  Wire.setClock(400000);

  if (develop::isDebugMode) {
    Serial.println("I2C: OK");
    Serial.println("Initializing I2C Devices...");
  }


  // BNOの初期化
  bool isSucceededBeginBNO = sensor::bno.begin();

  if (isSucceededBeginBNO) {
    if (develop::isDebugMode) {
      Serial.println("  BNO: Succeed");
      Serial.print("    Mode: ");
      Serial.println(sensor::bno.getMode());
      Serial.print("    System Status: ");
      Serial.println(sensor::bno.getSystemStatus());
      Serial.print("    Self Test: ");
      Serial.println(sensor::bno.getSelfTestResult());
      Serial.print("    System Error: ");
      Serial.println(sensor::bno.getSystemError());
      Serial.print("    Calibration: ");
      Serial.println(sensor::bno.isFullyCalibrated() ? "Done" : "Not yet");
    }
  }
  else {
    if (develop::isDebugMode) Serial.println("BNO: Failed");
  }


  // BMEの初期化
  bool isSucceededBeginBME = sensor::bme.begin();

  if (isSucceededBeginBME) {
    if (develop::isDebugMode) {
      Serial.println("  BME: Succeed");
      Serial.print("    Mode: ");
      Serial.println(sensor::bme.getMode());
    }
  }
  else {
    if (develop::isDebugMode) Serial.println("BME: Failed");
  }

  // サーミスタの初期化
  if (develop::isDebugMode) {
    Serial.println("--------------------------------------------------------------------------------");
    Serial.println("Initializing Thermistor...");
  }

  sensor::thermistor.initialize();


  // CANの初期化
  if (develop::isDebugMode) {
    Serial.println("--------------------------------------------------------------------------------");
    Serial.println("Initializing CAN-BUS...");
  }

  canbus::initialize();

  // タスクの初期化
  if (develop::isDebugMode) {
    Serial.println("--------------------------------------------------------------------------------");
    Serial.println("Registering Routines...");
  }

  Tasks.add(timer::task10Hz)->startIntervalMsec(100);
  Tasks.add(timer::task20Hz)->startIntervalMsec(50);
  Tasks.add(timer::task100Hz)->startIntervalMsec(10);

  if (develop::isDebugMode) {
    Serial.println("--------------------------------------------------------------------------------");
    Serial.println("Initialization is Done");
    Serial.println("Starting Routine");
  }

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
    indicator::sdStatus.on();
  }

  // SDが検知できなくなった時
  if (!recorder::doRecording && recorder::sd.isRunning() && recorder::cardDetection.isOpen()) {
    recorder::sd.end();
    Tasks.add("invalidSdBlink", timer::invalidSdBlink)->startIntervalMsec(500);
  }

  if (can.available0()) {
    CANMessage message;
    can.receive0(message);

    switch (message.id) {
    case static_cast<uint32_t>(canbus::Id::STATUS):
      canbus::receiveStatus(message);
      break;
    }
  }
}


void canbus::initialize() {
  ACAN_STM32_Settings settings(1000000);
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  can.begin(settings);
}


void canbus::sendScalar(canbus::Id id, float value) {
  CANMessage message;
  message.id = static_cast<uint32_t>(id);
  message.len = 4;

  canbus::converter.value = value;
  message.data[0] = canbus::converter.data[0];
  message.data[1] = canbus::converter.data[1];
  message.data[2] = canbus::converter.data[2];
  message.data[3] = canbus::converter.data[3];

  can.tryToSendReturnStatus(message);

  indicator::canSend.toggle();
}


void canbus::sendVector(canbus::Id id, canbus::Axis axis, float value) {
  CANMessage message;
  message.id = static_cast<uint32_t>(id);
  message.len = 5;
  message.data[0] = static_cast<uint8_t>(axis);

  canbus::converter.value = value;
  message.data[1] = canbus::converter.data[0];
  message.data[2] = canbus::converter.data[1];
  message.data[3] = canbus::converter.data[2];
  message.data[4] = canbus::converter.data[3];

  can.tryToSendReturnStatus(message);

  indicator::canSend.toggle();
}


void canbus::receiveStatus(CANMessage message) {
  flightMode::Mode mode = static_cast<flightMode::Mode>(message.data[0]);

  recorder::doRecording =
    mode == flightMode::Mode::STANDBY
    || mode == flightMode::Mode::THRUST
    || mode == flightMode::Mode::CLIMB
    || mode == flightMode::Mode::DESCENT
    || mode == flightMode::Mode::DECEL
    || mode == flightMode::Mode::PARACHUTE
    || mode == flightMode::Mode::LAND;

  indicator::canReceive.toggle();
}


void timer::task10Hz() {
  sensor::thermistor.getTemperature(&data::temperature);
  canbus::sendScalar(canbus::Id::TEMPERATURE, data::temperature);
}


void timer::task20Hz() {
  sensor::bno.getMagnetometer(&data::magnetometer_x, &data::magnetometer_y, &data::magnetometer_z);
  canbus::sendVector(canbus::Id::MAGNETOMETER, canbus::Axis::X, data::magnetometer_x);
  canbus::sendVector(canbus::Id::MAGNETOMETER, canbus::Axis::Y, data::magnetometer_y);
  canbus::sendVector(canbus::Id::MAGNETOMETER, canbus::Axis::Z, data::magnetometer_z);
}


void timer::task100Hz() {
  sensor::bno.getAcceleration(&data::acceleration_x, &data::acceleration_y, &data::acceleration_z);
  canbus::sendVector(canbus::Id::ACCELERATION, canbus::Axis::X, data::acceleration_x);
  canbus::sendVector(canbus::Id::ACCELERATION, canbus::Axis::Y, data::acceleration_y);
  canbus::sendVector(canbus::Id::ACCELERATION, canbus::Axis::Z, data::acceleration_z);

  sensor::bno.getGyroscope(&data::gyroscope_x, &data::gyroscope_y, &data::gyroscope_z);
  canbus::sendVector(canbus::Id::GYROSCOPE, canbus::Axis::X, data::gyroscope_x);
  canbus::sendVector(canbus::Id::GYROSCOPE, canbus::Axis::Y, data::gyroscope_y);
  canbus::sendVector(canbus::Id::GYROSCOPE, canbus::Axis::Z, data::gyroscope_z);

  sensor::bno.getOrientation(&data::orientation_x, &data::orientation_y, &data::orientation_z);
  canbus::sendVector(canbus::Id::ORIENTATION, canbus::Axis::X, data::orientation_x);
  canbus::sendVector(canbus::Id::ORIENTATION, canbus::Axis::Y, data::orientation_y);
  canbus::sendVector(canbus::Id::ORIENTATION, canbus::Axis::Z, data::orientation_z);

  sensor::bno.getLinearAcceleration(&data::linear_acceleration_x, &data::linear_acceleration_y, &data::linear_acceleration_z);
  canbus::sendVector(canbus::Id::LINEAR_ACCELERATION, canbus::Axis::X, data::linear_acceleration_x);
  canbus::sendVector(canbus::Id::LINEAR_ACCELERATION, canbus::Axis::Y, data::linear_acceleration_y);
  canbus::sendVector(canbus::Id::LINEAR_ACCELERATION, canbus::Axis::Z, data::linear_acceleration_z);

  sensor::bno.getGravityVector(&data::gravity_x, &data::gravity_y, &data::gravity_z);
  canbus::sendVector(canbus::Id::GRAVITY, canbus::Axis::X, data::gravity_x);
  canbus::sendVector(canbus::Id::GRAVITY, canbus::Axis::Y, data::gravity_y);
  canbus::sendVector(canbus::Id::GRAVITY, canbus::Axis::Z, data::gravity_z);

  sensor::bme.getPressure(&data::pressure);
  canbus::sendScalar(canbus::Id::PRESSURE, data::pressure);

  data::altitude = (((pow((data::referencePressure / data::pressure), (1.0 / 5.257))) - 1.0) * (data::temperature + 273.15)) / 0.0065;
  canbus::sendScalar(canbus::Id::ALTITUDE, data::altitude);
}


void timer::invalidSdBlink() {
  indicator::sdStatus.toggle();
}