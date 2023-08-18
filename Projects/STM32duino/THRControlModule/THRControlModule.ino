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
    double value;
    uint8_t data[8];
} converter;

// union Converter
// {
//     double correctedTemperature, coldJunctiontemperature, thermoCoupletemperature;
//     uint8_t correctedTemperatureData[8], coldJunctiontemperatureData[8], thermoCoupletemperatureData[8];
// } converter;



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

uint32_t referenceTime;
// float currentRate;
// uint32_t

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
                  task23Hz();

                //   Serial.print(F("CorrectedTemperature: "));
                // Serial.print("Temp: ");
                //   Serial.println(CorrectedTemperature());
                //   Serial.print(thermocouple.readError());

                
                  //   Serial.println(F(" | "));
                  //   performance(currentTime, currentRate);
                  //   uint32_t currentTime = millis();
                  //   performance(currentTime, taskRate());
                  //   Serial.print(F("ColdJunctionTemperature: "));
                  //   Serial.print(thermocouple.readInternal());
                  //   Serial.print(F(" | "));
                  //   Serial.print(F("Temperature: "));
                  //   Serial.print(thermocouple.readCelsius());
                  //   Serial.println(F(" | "));
              })
        ->startFps(23);

    Tasks.add([]()
            {
              uint32_t currentTime = millis();
              float currentRate = taskRate();
              performance(currentTime, currentRate);
            })
        ->startFps(144);
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

void performance(uint32_t currentTime, float taskRate)
{
    uint8_t data[8];
    memcpy(data, &currentTime, sizeof(currentTime));
    memcpy(data + sizeof(currentTime), &taskRate, sizeof(taskRate));

    CAN.sendMsgBuf(0x10D, 0, 8, data);

    memcpy(&currentTime, data, sizeof(currentTime));
    memcpy(&taskRate, data + sizeof(taskRate), sizeof(taskRate));

    // Serial.print(currentTime);
    // Serial.print(" | ");
    // Serial.println(taskRate);
}

float taskRate()
{
    uint32_t time = millis();
    float dataRate = 1000.0 / (float)(time - referenceTime);
    referenceTime = time;
    return dataRate;
}

void task23Hz()
{
    float getInternal = thermocouple.readInternal();
    float getCelsius = thermocouple.readInternal(); 
    float getCorrectedtemperature = CorrectedTemperature();

    canSendinternal(getInternal);
    canSendcelsius(getCelsius);
    canSendcorrectedTemperature(getCorrectedtemperature);
    



    // converter.value = CorrectedTemperature();
    // CAN.sendMsgBuf(0x100, 0, 8, converter.data);
    // converter.value = thermocouple.readInternal();
    // CAN.sendMsgBuf(0x101, 0, 8, converter.data);
    // converter.value = thermocouple.readCelsius();
    // CAN.sendMsgBuf(0x102, 0, 8, converter.data);

    // Serial.print("Temp: ");
    // Serial.println(thermocouple.readCelsius());
    // Serial.print("CorrectedTemp: ");
    // Serial.println(CorrectedTemperature());

    // double doubleData = 1010101;
    // memcpy(&doubleData, converter.data, 8);
    // Serial.print("converter.data: ");
    // Serial.println(doubleData);
}

void canSendinternal(float getInternal)
{
    uint8_t data[8];
    memcpy(data, &getInternal, sizeof(getInternal));

    Serial.print("Internal: ");
    Serial.print(getInternal);
    Serial.print(" | ");

    CAN.sendMsgBuf(0x101, 0, 8, data);
}

void canSendcelsius(float getCelsius)
{
    uint8_t data[8];
    memcpy(data, &getCelsius, sizeof(getCelsius));

    Serial.print("Celsius: ");
    Serial.print(getCelsius);
    Serial.print(" | ");

    CAN.sendMsgBuf(0x102, 0, 8, data);
}

void canSendcorrectedTemperature(float getCorrectedtemperature)
{
    uint8_t data[8];
    memcpy(data, &getCorrectedtemperature, sizeof(getCorrectedtemperature));

    Serial.print("CoTemp: ");
    Serial.print(getCorrectedtemperature);
    Serial.println(" | ");
    
    CAN.sendMsgBuf(0x103, 0, 8, data);
}