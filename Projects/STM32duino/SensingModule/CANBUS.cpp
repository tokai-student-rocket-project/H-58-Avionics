#include "CANBUS.hpp"


void CANBUS::initialize() {
  BITRATE bitrate = CAN_1000KBPS;
  int remap = 0;

  // Reference manual
  // https://www.st.com/content/ccc/resource/technical/document/reference_manual/4a/19/6e/18/9d/92/43/32/DM00043574.pdf/files/DM00043574.pdf/jcr:content/translations/en.DM00043574.pdf

  RCC->APB1ENR |= 0x2000000UL;          // Enable CAN clock 

  if (remap == 0) {
    RCC->AHBENR |= 0x20000UL;           // Enable GPIOA clock 
    setGpio(GPIOA, 11, AF9, 3);         // Set PA11 to AF9
    setGpio(GPIOA, 12, AF9, 3);         // Set PA12 to AF9
  }

  if (remap == 2) {
    RCC->AHBENR |= 0x40000UL;           // Enable GPIOB clock 
    setGpio(GPIOB, 8, AF9, 3);          // Set PB8 to AF9
    setGpio(GPIOB, 9, AF9, 3);          // Set PB9 to AF9
  }

  if (remap == 3) {
    RCC->AHBENR |= 0x100000UL;          // Enable GPIOD clock 
    setGpio(GPIOD, 0, AF7, 3);          // Set PD0 to AF7
    setGpio(GPIOD, 1, AF7, 3);          // Set PD1 to AF7
  }

  CAN1->MCR |= 0x1UL;                   // Set CAN to Initialization mode 
  while (!(CAN1->MSR & 0x1UL));         // Wait for Initialization mode

  //CAN1->MCR = 0x51UL;                 // Hardware initialization(No automatic retransmission)
  CAN1->MCR = 0x41UL;                   // Hardware initialization(With automatic retransmission)

  // Set bit rates 
  //CAN1->BTR &= ~(((0x03) << 24) | ((0x07) << 20) | ((0x0F) << 16) | (0x1FF)); 
  //CAN1->BTR |=  (((can_configs[bitrate].TS2-1) & 0x07) << 20) | (((can_configs[bitrate].TS1-1) & 0x0F) << 16) | ((can_configs[bitrate].BRP-1) & 0x1FF);
  CAN1->BTR &= ~(((0x03) << 24) | ((0x07) << 20) | ((0x0F) << 16) | (0x3FF));
  CAN1->BTR |= (((can_configs[bitrate].TS2 - 1) & 0x07) << 20) | (((can_configs[bitrate].TS1 - 1) & 0x0F) << 16) | ((can_configs[bitrate].BRP - 1) & 0x3FF);
  printRegister("CAN1->BTR=", CAN1->BTR);

  // Configure Filters to default values
  CAN1->FMR |= 0x1UL;                // Set to filter initialization mode
  CAN1->FMR &= 0xFFFFC0FF;             // Clear CAN2 start bank

  // bxCAN has 28 filters.
  // These filters are used for both CAN1 and CAN2.
  // STM32F303 has only CAN1, so all 28 are used for CAN1
  CAN1->FMR |= 0x1C << 8;              // Assign all filters to CAN1

  // Set fileter 0
  // Single 32-bit scale configuration 
  // Two 32-bit registers of filter bank x are in Identifier Mask mode
  // Filter assigned to FIFO 0 
  // Filter bank register to all 0
  setFilter(0, 1, 0, 0, 0x0UL, 0x0UL);

  CAN1->FMR &= ~(0x1UL);              // Deactivate initialization mode

  uint16_t TimeoutMilliseconds = 1000;
  bool can1 = false;
  CAN1->MCR &= ~(0x1UL);              // Require CAN1 to normal mode 

  // Wait for normal mode
  // If the connection is not correct, it will not return to normal mode.
  for (uint16_t wait_ack = 0; wait_ack < TimeoutMilliseconds; wait_ack++) {
    if ((CAN1->MSR & 0x1UL) == 0) {
      can1 = true;
      break;
    }
    delayMicroseconds(1000);
  }
  //Serial.print("can1=");
  //Serial.println(can1);
  if (can1) {
    Serial.println("CAN1 initialize ok");
  }
  else {
    Serial.println("CAN1 initialize fail!!");
  }
}


void CANBUS::sendMessage() {
  CAN_msg_t CAN_TX_msg;

  CAN_TX_msg.data[0] = 0x00;
  CAN_TX_msg.data[1] = 0x01;
  CAN_TX_msg.data[2] = 0x02;
  CAN_TX_msg.data[3] = 0x03;
  CAN_TX_msg.data[4] = 0x04;
  CAN_TX_msg.data[5] = 0x05;
  CAN_TX_msg.data[6] = 0x06;
  CAN_TX_msg.data[7] = 0x07;
  CAN_TX_msg.len = 8;

  CAN_TX_msg.type = DATA_FRAME;
  CAN_TX_msg.format = EXTENDED_FORMAT;
  CAN_TX_msg.id = 0xFF;

  //  --------------------------------------------------------------------------------------------------
  volatile int count = 0;

  uint32_t out = 0;
  if (CAN_TX_msg.format == EXTENDED_FORMAT) { // Extended frame format
    out = ((CAN_TX_msg.id & CAN_EXT_ID_MASK) << 3U) | STM32_CAN_TIR_IDE;
  }
  else {                                  // Standard frame format
    out = ((CAN_TX_msg.id & CAN_STD_ID_MASK) << 21U);
  }

  // Remote frame
  if (CAN_TX_msg.type == REMOTE_FRAME) {
    out |= STM32_CAN_TIR_RTR;
  }

  CAN1->sTxMailBox[0].TDTR &= ~(0xF);
  CAN1->sTxMailBox[0].TDTR |= CAN_TX_msg.len & 0xFUL;

  CAN1->sTxMailBox[0].TDLR = (((uint32_t)CAN_TX_msg.data[3] << 24) |
    ((uint32_t)CAN_TX_msg.data[2] << 16) |
    ((uint32_t)CAN_TX_msg.data[1] << 8) |
    ((uint32_t)CAN_TX_msg.data[0]));
  CAN1->sTxMailBox[0].TDHR = (((uint32_t)CAN_TX_msg.data[7] << 24) |
    ((uint32_t)CAN_TX_msg.data[6] << 16) |
    ((uint32_t)CAN_TX_msg.data[5] << 8) |
    ((uint32_t)CAN_TX_msg.data[4]));

  // Send Go
  CAN1->sTxMailBox[0].TIR = out | STM32_CAN_TIR_TXRQ;

  // Wait until the mailbox is empty
  while (CAN1->sTxMailBox[0].TIR & 0x1UL && count++ < 1000000);

  // The mailbox don't becomes empty while loop
  if (CAN1->sTxMailBox[0].TIR & 0x1UL) {
    Serial.println("Send Fail");
    Serial.println(CAN1->ESR, BIN);
    Serial.println(CAN1->MSR, BIN);
    Serial.println(CAN1->TSR, BIN);
  }
}


void CANBUS::setGpio(GPIO_TypeDef* addr, uint8_t index, uint8_t afry, uint8_t speed) {
  uint8_t _index2 = index * 2;
  uint8_t _index4 = index * 4;
  uint8_t ofs = 0;
  uint8_t setting;

  if (index > 7) {
    _index4 = (index - 8) * 4;
    ofs = 1;
  }

  uint32_t mask;
  printRegister("GPIO_AFR(b)=", addr->AFR[1]);
  mask = 0xF << _index4;
  addr->AFR[ofs] &= ~mask;         // Reset alternate function
  //setting = 0x9;                    // AF9
  setting = afry;                   // Alternative function selection
  mask = setting << _index4;
  addr->AFR[ofs] |= mask;          // Set alternate function
  printRegister("GPIO_AFR(a)=", addr->AFR[1]);

  printRegister("GPIO_MODER(b)=", addr->MODER);
  mask = 0x3 << _index2;
  addr->MODER &= ~mask;           // Reset mode
  setting = 0x2;                    // Alternate function mode
  mask = setting << _index2;
  addr->MODER |= mask;            // Set mode
  printRegister("GPIO_MODER(a)=", addr->MODER);

  printRegister("GPIO_OSPEEDR(b)=", addr->OSPEEDR);
  mask = 0x3 << _index2;
  addr->OSPEEDR &= ~mask;           // Reset speed
  setting = speed;
  mask = setting << _index2;
  addr->OSPEEDR |= mask;            // Set speed
  printRegister("GPIO_OSPEEDR(a)=", addr->OSPEEDR);

  printRegister("GPIO_OTYPER(b)=", addr->OTYPER);
  mask = 0x1 << index;
  addr->OTYPER &= ~mask;           // Reset Output push-pull
  printRegister("GPIO_OTYPER(a)=", addr->OTYPER);

  printRegister("GPIO_PUPDR(b)=", addr->PUPDR);
  mask = 0x3 << _index2;
  addr->PUPDR &= ~mask;           // Reset port pull-up/pull-down
  printRegister("GPIO_PUPDR(a)=", addr->PUPDR);
}


void CANBUS::setFilter(uint8_t index, uint8_t scale, uint8_t mode, uint8_t fifo, uint32_t bank1, uint32_t bank2) {
  if (index > 27) return;

  CAN1->FA1R &= ~(0x1UL << index);               // Deactivate filter

  if (scale == 0) {
    CAN1->FS1R &= ~(0x1UL << index);             // Set filter to Dual 16-bit scale configuration
  }
  else {
    CAN1->FS1R |= (0x1UL << index);              // Set filter to single 32 bit configuration
  }
  if (mode == 0) {
    CAN1->FM1R &= ~(0x1UL << index);             // Set filter to Mask mode
  }
  else {
    CAN1->FM1R |= (0x1UL << index);              // Set filter to List mode
  }

  if (fifo == 0) {
    CAN1->FFA1R &= ~(0x1UL << index);            // Set filter assigned to FIFO 0
  }
  else {
    CAN1->FFA1R |= (0x1UL << index);             // Set filter assigned to FIFO 1
  }

  CAN1->sFilterRegister[index].FR1 = bank1;    // Set filter bank registers1
  CAN1->sFilterRegister[index].FR2 = bank2;    // Set filter bank registers2

  CAN1->FA1R |= (0x1UL << index);                // Activate filter

}


void CANBUS::printRegister(char* buf, uint32_t reg) {
  Serial.print(buf);
  Serial.print("0x");
  Serial.print(reg, BIN);
  Serial.println();
}