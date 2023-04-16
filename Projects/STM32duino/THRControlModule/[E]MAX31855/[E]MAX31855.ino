#include <SPI.h>
#include "Adafruit_MAX31855.h"

#define MAXDO 3
#define MAXCS 4
#define MAXCLK 5
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// #define MAXCS 10

// Adafruit_MAX31855 thermocouple(MAXCS, SPI);
// Adafruit_MAX31855 thermocouple(MAXCS, MAXDO);

void setup()
{
    Serial2.begin(9600);
    // SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
    // SPISettings(5000000, MSBFIRST, SPI_MODE0);
    // SPI.begin();
    

    while (!Serial2)
        delay(100);

    Serial2.println("MAX31855 Test...");
    delay(500);
    Serial2.print("Initializing sensor...");
    if (!thermocouple.begin())
    {
        Serial2.println("ERROR");
        while (1)
            delay(100);
    }
    Serial2.println("DONE");
}

void loop()
{
    Serial2.print("Internal Temp = ");
    Serial2.println(thermocouple.readInternal());

    double c = thermocouple.readCelsius();
    if(isnan(c))
    {
        Serial2.println("Thermocouple fault(s) detected!");
        uint8_t e = thermocouple.readError();
        if (e & MAX31855_FAULT_OPEN)Serial2.println("FAULT: Termocouple is open -- no connections.");
        if (e & MAX31855_FAULT_SHORT_GND)Serial2.println("FAULT: Thermocouple is short --circuited to GND");
        if (e & MAX31855_FAULT_SHORT_VCC)Serial2.println("FAULT: Thermocouple is short --circuited to VCC");
    }else{
        Serial2.print("c = ");
        Serial2.println(c);
    }

    delay(1000);
}