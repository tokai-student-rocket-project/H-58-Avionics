#ifndef CAN_BUS_HPP_
#define CAN_BUS_HPP_

#include <ACAN_STM32.h>
#include "DataType.hpp"


class CANBUS {
public:
  void initialize();
  void send(uint32_t id, double value);
  void sendVector(uint32_t id, raw_t x, raw_t y, raw_t z);
private:
};


#endif