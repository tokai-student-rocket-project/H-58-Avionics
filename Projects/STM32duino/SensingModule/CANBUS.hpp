#ifndef CAN_BUS_HPP_
#define CAN_BUS_HPP_

#include <ACAN_STM32.h>


class CANBUS {
public:
  void initialize();
  void send(uint32_t id, double value);
private:
};


#endif