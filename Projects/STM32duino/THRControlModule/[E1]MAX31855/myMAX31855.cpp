#include "myMAX31855.h"
MAX31855_Class::MAX31855_Class() {}
MAX31855_Class::~MAX31855_Class() {}

bool MAX31855_Class::begin(const uint8_t chipSelect, const bool reverse)
{
    _reversed = reverse;
    _cs = chipSelect;
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
    SPI.begin();
    readRaw();
    return (bool)_errorCode;
}

uint8_t MAX31855_Class::fault() const
{
    return _errorCode;
}

int32_t MAX31855_Class::readRaw()
{
    int32_t dataBuffer = 0;
    for (uint8_t retryCounter = 0; retryCounter < READING_RETRIES; retryCounter++)
    {
        digitalWrite(_cs, LOW);
        delayMicroseconds(SPI_DELAY_MICROSECONDS);
        if (_sck == 0)
        {
            SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
            dataBuffer = SPI.transfer(0);
            dataBuffer <<= 8;
            dataBuffer |= SPI.transfer(0);
            dataBuffer <<= 8;
            dataBuffer = SPI.transfer(0);
            dataBuffer <<= 8;
            dataBuffer |= SPI.transfer(0);
            SPI.endTransaction();
        }
        else
        {
            digitalWrite(_sck, LOW);
            delayMicroseconds(SPI_DELAY_MICROSECONDS);
            for (uint8_t i = 0; i < 32; i++)
            {
                digitalWrite(_sck, LOW);
                delayMicroseconds(SPI_DELAY_MICROSECONDS);
                dataBuffer <<= 1;
                if (digitalRead(_miso))
                    dataBuffer |= 1;
                digitalWrite(_sck, HIGH);
                delayMicroseconds(SPI_DELAY_MICROSECONDS);
            }
        }
        digitalWrite(_cs, HIGH);
        _errorCode = dataBuffer & B111;
        if (!_errorCode)
        {
            break;
        }
        delay(25);
    }
    return dataBuffer;
}
int32_t MAX31855_Class::readProbe()
{
    int32_t rawBuffer = readRaw();
    int32_t dataBuffer = rawBuffer;
    if (dataBuffer & B111)
        dataBuffer = INT32_MAX;
    else
    {
        dataBuffer = dataBuffer >> 18;
        if (dataBuffer & 0x2000)
            dataBuffer |= 0xFFFE000;
        dataBuffer *= (int32_t)250;
    }
    if (_reversed)
    {
        int32_t ambientBuffer = (rawBuffer & 0xFFFF) >> 4;
        if (ambientBuffer & 0x2000)
            ambientBuffer |= 0xFFFF000;
        ambientBuffer = ambientBuffer * (int32_t)625 / (int32_t)10;
        dataBuffer = (ambientBuffer - dataBuffer) + ambientBuffer;
    }
    return dataBuffer;
}

int32_t MAX31855_Class::readAmbient()
{
    int32_t dataBuffer = readRaw();
    if (dataBuffer & B111)
        dataBuffer = INT32_MAX;
    else
    {
        dataBuffer = (dataBuffer & 0xFFFF) >> 4;
        if (dataBuffer & 0x2000)
            dataBuffer |= 0xFFFF000;
        dataBuffer = dataBuffer * (int32_t)625 / (int32_t)10;
    }
    return dataBuffer;
}