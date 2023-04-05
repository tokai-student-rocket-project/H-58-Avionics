#include <Wire.h>
#include <TaskManager.h>
#include "BNO055.hpp"


BNO055 bno055(&Wire, 0x28);


double acceleration_x, acceleration_y, acceleration_z;
double magnetometer_x, magnetometer_y, magnetometer_z;
double gyroscope_x, gyroscope_y, gyroscope_z;
double euler_heading, euler_roll, euler_pitch;
double quaternion_w, quaternion_x, quaternion_y, quaternion_z;
double linear_acceleration_x, linear_acceleration_y, linear_acceleration_z;
double gravity_x, gravity_y, gravity_z;


void setup() {
  Serial.begin(115200);

  Wire.setSDA(PB_7);
  Wire.setSCL(PB_6);
  Wire.begin();
  Wire.setClock(400000);

  bno055.initialize();

  Tasks.add(task20Hz)->startIntervalMsec(50);
  Tasks.add(task100Hz)->startIntervalMsec(10);
}


void loop() {
  Tasks.update();
}


void task20Hz() {
  bno055.getGravityVector(&magnetometer_x, &magnetometer_y, &magnetometer_z);
}


void task100Hz() {
  bno055.getAcceleration(&acceleration_x, &acceleration_y, &acceleration_z);
  bno055.getGyroscope(&gyroscope_x, &gyroscope_y, &gyroscope_z);
  bno055.getQuaternion(&quaternion_w, &quaternion_x, &quaternion_y, &quaternion_z);
  bno055.getLinearAcceleration(&linear_acceleration_x, &linear_acceleration_y, &linear_acceleration_z);
  bno055.getGravityVector(&gravity_x, &gravity_y, &gravity_z);

  Serial.print(linear_acceleration_x); Serial.print(",");
  Serial.print(linear_acceleration_y); Serial.print(",");
  Serial.println(linear_acceleration_z);
}