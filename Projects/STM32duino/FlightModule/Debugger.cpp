#include "Debugger.hpp"


void Debugger::initialize() {
  Serial.begin(115200);
}


void Debugger::printMessage(const String& message) {
  Serial.println(message);
}


void Debugger::printScalar(const String& label, float value) {
  Serial.print(label);
  Serial.print(":");
  Serial.println(value);
}


void Debugger::printVector(const String& label, float x, float y, float z) {
  Serial.print(label);
  Serial.print("_x:");
  Serial.print(x);
  Serial.print(",");
  Serial.print(label);
  Serial.print("_y:");
  Serial.print(y);
  Serial.print(",");
  Serial.print(label);
  Serial.print("_z:");
  Serial.println(z);
}