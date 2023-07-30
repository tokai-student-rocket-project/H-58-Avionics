#include <TaskManager.h>
#include "IcsHardSerialClass.h"
#include "Adafruit_MAX31855.h"
#include <SPI.h>
#include <mcp2515_can.h>

/* CAN Config START */
const int SPI_CS_PIN = 6;
mcp2515_can CAN(SPI_CS_PIN);
// unsigned char sample[8] = {0, 0, 0, 0, 0, 0, 0, 0};

union Converter
{
    double correctedTemperature, coldJunctiontemperature, thermoCoupletemperature;
    uint8_t correctedTemperatureData[8], coldJunctiontemperatureData[8], thermoCoupletemperatureData[8];
} converter;

/* CAN Config END */

/*MAX31855 Config START*/

Adafruit_MAX31855 thermocouple(5);

/*MAX31855 Config END*/

/*MAX31855 Pin Config*/
// MAX31855      Every
// CLK  -------- SCK(D13)
// CS   -------- CS_Pin(D3)
// DO   -------- CIPO(D12)
// GND  -------- GND
// 3Vo  -------- N/C
// Vin  -------- 3~5V

void setup()
{
    Serial.begin(115200);

    /* --- CAN --- */

    CAN.begin(CAN_1000KBPS, MCP_8MHz); // 250kbps　<- 250kbpsに設定, MCP_8MHz <- MCP2515に搭載する水晶発振子に依存 //この値に固定

    /* --- CAN Config END --- */

    /* --- MAX31855 ---*/

    thermocouple.begin();

    /* ---  MAX31855 Config END ---*/

    Tasks.add("task", []()
              {
                  converter.correctedTemperature = CorrectedTemperature();
                  CAN.sendMsgBuf(0x100, 0, 8, converter.correctedTemperatureData);
                  converter.coldJunctiontemperature = thermocouple.readInternal();
                  CAN.sendMsgBuf(0x101, 0, 8, converter.coldJunctiontemperatureData);
                  converter.thermoCoupletemperature = thermocouple.readCelsius();
                  CAN.sendMsgBuf(0x102, 0, 8, converter.thermoCoupletemperatureData);
                  Serial.print(F("CorrectedTemperature: "));
                  Serial.print(CorrectedTemperature());
                  Serial.print(F(" | "));
                  Serial.print(F("ColdJunctionTemperature: "));
                  Serial.print(thermocouple.readInternal());
                  Serial.print(F(" | "));
                  Serial.print(F("Temperature: "));
                  Serial.print(thermocouple.readCelsius());
                  Serial.print(F(" | ")); })
        ->startFps(23);
}

void loop()
{
    Tasks.update();
}

float CorrectedTemperature()
{
    const float VOLTAGE_PER_DEGREE = 41.276;
    float correcttemperature;

    float temperature = thermocouple.readCelsius();
    float coldtemperature = thermocouple.readInternal();
    float Voltage = VOLTAGE_PER_DEGREE * (temperature - coldtemperature);

    return correcttemperature = Voltage / VOLTAGE_PER_DEGREE + coldtemperature;
}