#include <Wire.h>
#include <TaskManager.h>
#include <ACAN_STM32.h>
#include "BNO055.hpp"
#include "Thermistor.hpp"

#define ARDUINO_NUCLEO_F303K8


static uint32_t gSendDate = 0;
static uint32_t gSentCount = 0;
static uint32_t gReceivedCount = 0;


BNO055 bno055(&Wire, 0x28);
double acceleration_x, acceleration_y, acceleration_z;
double magnetometer_x, magnetometer_y, magnetometer_z;
double gyroscope_x, gyroscope_y, gyroscope_z;
double euler_heading, euler_roll, euler_pitch;
double quaternion_w, quaternion_x, quaternion_y, quaternion_z;
double linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
double gravity_x, gravity_y, gravity_z;

Thermistor thermistor1(PA_2);
double temperature1;

Thermistor thermistor2(PA_3);
double temperature2;


void setup() {
  analogReadResolution(12);

  Serial.begin(115200);

  Wire.setSDA(PB_7);
  Wire.setSCL(PB_6);
  Wire.begin();
  Wire.setClock(400000);

  bno055.initialize();
  thermistor1.initialize();
  thermistor2.initialize();

  pinMode(LED_BUILTIN, OUTPUT);

  ACAN_STM32_Settings settings(1000 * 1000);

  Serial.print("Bit Rate prescaler: ");
  Serial.println(settings.mBitRatePrescaler);
  Serial.print("Phase segment 1: ");
  Serial.println(settings.mPhaseSegment1);
  Serial.print("Phase segment 2: ");
  Serial.println(settings.mPhaseSegment2);
  Serial.print("RJW: ");
  Serial.println(settings.mRJW);
  Serial.print("Actual Bit Rate: ");
  Serial.print(settings.actualBitRate());
  Serial.println(" bit/s");
  Serial.print("Sample point: ");
  Serial.print(settings.samplePointFromBitStart());
  Serial.println("%");
  Serial.print("Exact Bit Rate ? ");
  Serial.println(settings.exactBitRate() ? "yes" : "no");

  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;

  const uint32_t errorCode = can.begin(settings);
  if (0 == errorCode) {
    Serial.println("can ok");
  }
  else {
    Serial.print("Error can: 0x");
    Serial.println(errorCode, HEX);
  }

  Tasks.add(task1Hz)->startIntervalMsec(1000);
  Tasks.add(task10Hz)->startIntervalMsec(100);
  Tasks.add(task20Hz)->startIntervalMsec(50);
  Tasks.add(task100Hz)->startIntervalMsec(10);
}


void loop() {
  Tasks.update();
}


void task1Hz() {
  CANMessage message;
  message.id = 0x542;
  message.len = 8;
  message.data[0] = 0;
  message.data[1] = 1;
  message.data[2] = 2;
  message.data[3] = 3;
  message.data[4] = 4;
  message.data[5] = 5;
  message.data[6] = 6;
  message.data[7] = 7;
  const bool ok = can.tryToSendReturnStatus(message);
  if (ok) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    gSendDate += 1000;
    gSentCount += 1;
    Serial.print("Sent: ");
    Serial.println(gSentCount);
  }
}


void task10Hz() {
  thermistor1.getTemperature(&temperature1);
  thermistor2.getTemperature(&temperature2);
}


void task20Hz() {
  bno055.getMagnetometer(&magnetometer_x, &magnetometer_y, &magnetometer_z);
}


void task100Hz() {
  bno055.getAcceleration(&acceleration_x, &acceleration_y, &acceleration_z);
  bno055.getGyroscope(&gyroscope_x, &gyroscope_y, &gyroscope_z);
  bno055.getEuler(&euler_heading, &euler_roll, &euler_pitch);
  bno055.getQuaternion(&quaternion_w, &quaternion_x, &quaternion_y, &quaternion_z);
  bno055.getLinearAcceleration(&linear_acceleration_x, &linear_acceleration_y, &linear_acceleration_z);
  bno055.getGravityVector(&gravity_x, &gravity_y, &gravity_z);
}