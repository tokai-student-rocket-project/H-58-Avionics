#ifndef CAN_BUS_HPP_
#define CAN_BUS_HPP_

#include <Arduino.h>
#include <stm32f3xx_ll_bus.h>

#define CAN_FILTER_SCALE_16BIT 0x00U
#define CAN_FILTER_SCALE_32BIT 0x01U
#define CAN_FILTER_MODE_IDMASK 0x00U
#define CAN_FILTER_MODE_IDLIST 0x01U
#define CAN_FILTER_FIFO0 0x00U
#define CAN_FILTER_FIFO1 0x01U

#define CAN_RTR_DATA 0x00U // Data frame
#define CAN_RTR_REMOTE 0x02U // Remote frame
#define CAN_ID_STD 0x00U // Standard Id
#define CAN_ID_EXT 0x04U // Extended Id
#define CAN_RXFIFO0 0x00 // specification RXFIFO0
#define CAN_RXFIFO1 0x01 // specification RXFIFO1


class CANBUS {
public:
  void initialize();

  void send();

private:
  void initializeMsp();
  void initializeParameters();
  void initializeFilterParameters();
  void start();

  uint32_t CAN_GetTxMailboxesFreeLevel(CAN_TypeDef* CANx);
  void CAN_RX0_IRQHandler();
  void CAN_RX1_IRQHandler();
};


#endif