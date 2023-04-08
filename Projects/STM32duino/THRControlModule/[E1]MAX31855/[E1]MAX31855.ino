/*
内部の補正用温度がおかしい。
熱電対から温度を読み出せていない。
*/

#include <SPI.h>
#include "myMAX31855.h"

const uint32_t SERIAL_SPEED{115200};
const uint8_t SPI_CHIP_SELECT{10};
const uint8_t SPI_MISO{MISO};
const uint8_t SPI_SYSTEM_CLOCK{SCK};

MAX31855_Class MAX31855;

void setup()
{
    Serial2.begin(SERIAL_SPEED);
    Serial.println(F("Initializing MAX31855 sensor\n"));
    while (!MAX31855.begin(SPI_CHIP_SELECT))
    {
        Serial.println(F("Unable to start MAX31855. Waiting 3 seconds."));
        delay(3000);
    }
    Serial.println();
}

void loop()
{
    int32_t ambientTemperature = MAX31855.readAmbient();
    int32_t probeTemperature = MAX31855.readProbe();
    uint8_t faultCode = MAX31855.fault();
    if (faultCode)
    {
        if (faultCode & B001)
        {
            Serial2.println(F("Fault: Wrire not connected"));
        }
        if (faultCode & B010)
        {
            Serial2.println(F("Fault: Short-circuited to Ground (negative)"));
        }
        if (faultCode & B100)
        {
            Serial2.println(F("Fault: Short-circuited to VCC(positive)"));
        }
    }
    else
    {
        Serial2.print("Ambient Temprature = ");
        Serial2.print((float)ambientTemperature / 1000, 3);
        Serial2.println("C");
        Serial2.print("Probe Temprature = ");
        Serial2.print((float)probeTemperature / 1000, 3);
        Serial.println("C\n");
    }
    delay(1000);
}
