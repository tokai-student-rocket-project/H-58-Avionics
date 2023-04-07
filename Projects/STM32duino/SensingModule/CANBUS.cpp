#include "CANBUS.hpp"


void CANBUS::initialize() {
  CAN_TypeDef* instance = CAN;
  uint32_t prescaler = 2;
  uint32_t mode = 0x00000000;
  uint32_t syncJumpWidth = 0x00000000;
  uint32_t timeSeg1 = CAN_BTR_TS1_2 | CAN_BTR_TS1_1 | CAN_BTR_TS1_0;
  uint32_t timeSeg2 = CAN_BTR_TS2_2 | CAN_BTR_TS2_1;
  FunctionalState timeTriggeredMode = DISABLE;
  FunctionalState autoBusOff = DISABLE;
  FunctionalState autoWakeUp = DISABLE;
  FunctionalState autoRetransmission = DISABLE;
  FunctionalState receiveFifoLocked = DISABLE;
  FunctionalState transmitFifoPriority = DISABLE;

  /* Request initialisation */
  SET_BIT(instance->MCR, CAN_MCR_INRQ);
  while ((instance->MSR & CAN_MSR_INAK) == 0U);

  /* Exit from sleep mode */
  CLEAR_BIT(instance->MCR, CAN_MCR_SLEEP);
  while ((instance->MSR & CAN_MSR_SLAK) != 0U);

  /* Set the time triggered communication mode */
  if (timeTriggeredMode == ENABLE) {
    SET_BIT(instance->MCR, CAN_MCR_TTCM);
  }
  else {
    CLEAR_BIT(instance->MCR, CAN_MCR_TTCM);
  }

  /* Set the automatic bus-off management */
  if (autoBusOff == ENABLE) {
    SET_BIT(instance->MCR, CAN_MCR_ABOM);
  }
  else {
    CLEAR_BIT(instance->MCR, CAN_MCR_ABOM);
  }

  /* Set the automatic wake-up mode */
  if (autoWakeUp == ENABLE) {
    SET_BIT(instance->MCR, CAN_MCR_AWUM);
  }
  else {
    CLEAR_BIT(instance->MCR, CAN_MCR_AWUM);
  }

  /* Set the automatic retransmission */
  if (autoRetransmission == ENABLE) {
    CLEAR_BIT(instance->MCR, CAN_MCR_NART);
  }
  else {
    SET_BIT(instance->MCR, CAN_MCR_NART);
  }

  /* Set the receive FIFO locked mode */
  if (receiveFifoLocked == ENABLE) {
    SET_BIT(instance->MCR, CAN_MCR_RFLM);
  }
  else {
    CLEAR_BIT(instance->MCR, CAN_MCR_RFLM);
  }

  /* Set the transmit FIFO priority */
  if (transmitFifoPriority == ENABLE) {
    SET_BIT(instance->MCR, CAN_MCR_TXFP);
  }
  else {
    CLEAR_BIT(instance->MCR, CAN_MCR_TXFP);
  }

  /* Set the bit timing register */
  WRITE_REG(instance->BTR, (uint32_t)(mode |
    syncJumpWidth |
    timeSeg1 |
    timeSeg2 |
    (prescaler - 1U)));


  uint32_t filterBank = 0;
  uint32_t filterMode = 0x00000001;
  uint32_t filterScale = 0x00000000;
  uint32_t filterIdHigh = 0x456 << 5;
  uint32_t filterIdLow = 0x456 << 5;
  uint32_t filterMaskIdHigh = 0x456 << 5;
  uint32_t filterMaskIdLow = 0x456 << 5;
  uint32_t filterFIFOAssignment = 0x00000000;
  uint32_t filterActivation = ENABLE;

  /* Initialisation mode for the filter */
  SET_BIT(instance->FMR, CAN_FMR_FINIT);

  /* Convert filter number into bit position */
  uint32_t filternbrbitpos;
  filternbrbitpos = (uint32_t)1 << (filterBank & 0x1FU);

  /* Filter Deactivation */
  CLEAR_BIT(instance->FA1R, filternbrbitpos);

  /* Filter Scale */
  if (filterScale == 0x00000000)
  {
    /* 16-bit scale for the filter */
    CLEAR_BIT(instance->FS1R, filternbrbitpos);

    /* First 16-bit identifier and First 16-bit mask */
    /* Or First 16-bit identifier and Second 16-bit identifier */
    instance->sFilterRegister[filterBank].FR1 =
      ((0x0000FFFFU & filterMaskIdLow) << 16U) |
      (0x0000FFFFU & filterIdLow);

    /* Second 16-bit identifier and Second 16-bit mask */
    /* Or Third 16-bit identifier and Fourth 16-bit identifier */
    instance->sFilterRegister[filterBank].FR2 =
      ((0x0000FFFFU & filterMaskIdHigh) << 16U) |
      (0x0000FFFFU & filterIdHigh);
  }

  if (filterScale == 0x00000001)
  {
    /* 32-bit scale for the filter */
    SET_BIT(instance->FS1R, filternbrbitpos);

    /* 32-bit identifier or First 32-bit identifier */
    instance->sFilterRegister[filterBank].FR1 =
      ((0x0000FFFFU & filterIdHigh) << 16U) |
      (0x0000FFFFU & filterIdLow);

    /* 32-bit mask or Second 32-bit identifier */
    instance->sFilterRegister[filterBank].FR2 =
      ((0x0000FFFFU & filterMaskIdHigh) << 16U) |
      (0x0000FFFFU & filterMaskIdLow);
  }

  /* Filter Mode */
  if (filterMode == 0x00000000)
  {
    /* Id/Mask mode for the filter*/
    CLEAR_BIT(instance->FM1R, filternbrbitpos);
  }
  else /* CAN_FilterInitStruct->CAN_FilterMode == CAN_FilterMode_IdList */
  {
    /* Identifier list mode for the filter*/
    SET_BIT(instance->FM1R, filternbrbitpos);
  }

  /* Filter FIFO assignment */
  if (filterFIFOAssignment == 0x00000000)
  {
    /* FIFO 0 assignation for the filter */
    CLEAR_BIT(instance->FFA1R, filternbrbitpos);
  }
  else
  {
    /* FIFO 1 assignation for the filter */
    SET_BIT(instance->FFA1R, filternbrbitpos);
  }

  /* Filter activation */
  if (filterActivation == 0x00000001)
  {
    SET_BIT(instance->FA1R, filternbrbitpos);
  }

  /* Leave the initialisation mode for the filter */
  CLEAR_BIT(instance->FMR, CAN_FMR_FINIT);


  /* Request leave initialisation */
  CLEAR_BIT(instance->MCR, CAN_MCR_INRQ);
  while ((instance->MSR & CAN_MSR_INAK) != 0U);
}


void CANBUS::send() {
  CAN_TypeDef* instance = CAN;

  uint32_t stdId = 1234;
  uint32_t extId;
  uint32_t rTR = 0x00000000;
  uint32_t iDE = 0x00000000;
  uint32_t dLC = 3;
  FunctionalState transmitGlobalTime = DISABLE;
  _txData[0] = 100;
  _txData[1] = 200;
  _txData[2] = 300;

  uint32_t tsr = READ_REG(instance->TSR);

  /* Check that all the Tx mailboxes are not full */
  if (((tsr & CAN_TSR_TME0) != 0U) ||
    ((tsr & CAN_TSR_TME1) != 0U) ||
    ((tsr & CAN_TSR_TME2) != 0U)) {
    /* Select an empty transmit mailbox */
    uint32_t transmitmailbox;
    transmitmailbox = (tsr & CAN_TSR_CODE) >> CAN_TSR_CODE_Pos;

    /* Set up the Id */
    if (iDE == 0x00000000) {
      instance->sTxMailBox[transmitmailbox].TIR = ((stdId << CAN_TI0R_STID_Pos) | rTR);
    }
    else {
      instance->sTxMailBox[transmitmailbox].TIR = ((extId << CAN_TI0R_EXID_Pos) | iDE | rTR);
    }

    /* Set up the DLC */
    instance->sTxMailBox[transmitmailbox].TDTR = (dLC);

    /* Set up the Transmit Global Time mode */
    if (transmitGlobalTime == ENABLE)
    {
      SET_BIT(instance->sTxMailBox[transmitmailbox].TDTR, CAN_TDT0R_TGT);
    }

    /* Set up the data field */
    WRITE_REG(instance->sTxMailBox[transmitmailbox].TDHR,
      ((uint32_t)_txData[7] << CAN_TDH0R_DATA7_Pos) |
      ((uint32_t)_txData[6] << CAN_TDH0R_DATA6_Pos) |
      ((uint32_t)_txData[5] << CAN_TDH0R_DATA5_Pos) |
      ((uint32_t)_txData[4] << CAN_TDH0R_DATA4_Pos));
    WRITE_REG(instance->sTxMailBox[transmitmailbox].TDLR,
      ((uint32_t)_txData[3] << CAN_TDL0R_DATA3_Pos) |
      ((uint32_t)_txData[2] << CAN_TDL0R_DATA2_Pos) |
      ((uint32_t)_txData[1] << CAN_TDL0R_DATA1_Pos) |
      ((uint32_t)_txData[0] << CAN_TDL0R_DATA0_Pos));

    /* Request transmission */
    SET_BIT(instance->sTxMailBox[transmitmailbox].TIR, CAN_TI0R_TXRQ);
  }
  else {
    return;
  }


  uint32_t freelevel = 0;

  /* Check Tx Mailbox 0 status */
  if ((instance->TSR & CAN_TSR_TME0) != 0U) {
    freelevel++;
  }

  /* Check Tx Mailbox 1 status */
  if ((instance->TSR & CAN_TSR_TME1) != 0U) {
    freelevel++;
  }

  /* Check Tx Mailbox 2 status */
  if ((instance->TSR & CAN_TSR_TME2) != 0U) {
    freelevel++;
  }

  while (freelevel != 3);
}