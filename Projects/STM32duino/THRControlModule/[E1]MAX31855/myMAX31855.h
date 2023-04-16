#include "Arduino.h"
#include <SPI.h>
#ifndef myMAX31855_h

#define myMAX31855_h

const uint16_t SPI_DELAY_MICROSECONDS{1000};
const uint8_t READING_RETRIES{64};

class MAX31855_Class
{
public:
    MAX31855_Class();
    ~MAX31855_Class();
    bool begin(const uint8_t chipSelect, const bool reverse = false);
    bool begin(const uint8_t chipSelect, const uint8_t miso, const uint8_t sck, const bool reverse = false);
    int32_t readProbe();
    int32_t readAmbient();
    uint8_t fault() const;

private:
    int32_t readRaw();
    uint8_t _cs = 0, _miso = 0, _sck = 0;
    uint8_t _errorCode = 0;
    bool _reversed = false;
};
#endif