#ifndef CAN_BUS_HPP_
#define CAN_BUS_HPP_

#include <Arduino.h>
#include <stm32f303x8.h>


class CANBUS {
public:
  void initialize();
  void send();

private:
  uint8_t _txData[8];
};


#endif