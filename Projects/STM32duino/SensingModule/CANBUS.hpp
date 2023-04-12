#ifndef CAN_BUS_HPP_
#define CAN_BUS_HPP_

#include <ACAN_STM32.h>


class CANBUS {
public:
  void initialize();
  void send(uint32_t id, double value);
  void sendVector(uint32_t id,
    uint8_t xLSB, uint8_t xMSB,
    uint8_t yLSB, uint8_t yMSB,
    uint8_t zLSB, uint8_t zMSB);
private:
};


#endif