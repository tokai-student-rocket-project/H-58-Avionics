#include "CANBUS.hpp"


void CANBUS::initialize() {
  initializeMsp();
  initializeParameters();
  initializeFilterParameters();
  start();
}


void CANBUS::send() {
  uint8_t TxData[8];

  TxData[0] = 0x00;
  TxData[1] = 0x01;
  TxData[2] = 0x02;
  TxData[3] = 0x03;
  TxData[4] = 0x04;
  TxData[5] = 0x05;
  TxData[6] = 0x06;
  TxData[7] = 0x07;

  uint32_t stdId = 0x01;
  uint32_t rtr = CAN_RTR_DATA;
  uint32_t id = CAN_ID_STD;
  uint32_t length = 8;

  uint32_t transmitmailbox;
  uint32_t tsr;
  tsr = READ_REG(CAN->TSR);

  if (((tsr & CAN_TSR_TME0) != 0U) || ((tsr & CAN_TSR_TME1) != 0U) || ((tsr & CAN_TSR_TME2) != 0U)) {
    transmitmailbox = (tsr & CAN_TSR_CODE) >> CAN_TSR_CODE_Pos;

    // データボックスに2つ以上データが入っていた場合はエラーを返す
    if (transmitmailbox > 2U) return;

    CAN->sTxMailBox[transmitmailbox].TIR = ((stdId << CAN_TI0R_STID_Pos) | rtr);
    // データの長さを設定
    CAN->sTxMailBox[transmitmailbox].TDTR = (length);

    // データをメールボックスに投げ込む
    WRITE_REG(CAN->sTxMailBox[transmitmailbox].TDHR,
      ((uint32_t)TxData[7] << CAN_TDH0R_DATA7_Pos) |
      ((uint32_t)TxData[6] << CAN_TDH0R_DATA6_Pos) |
      ((uint32_t)TxData[5] << CAN_TDH0R_DATA5_Pos) |
      ((uint32_t)TxData[4] << CAN_TDH0R_DATA4_Pos));
    WRITE_REG(CAN->sTxMailBox[transmitmailbox].TDLR,
      ((uint32_t)TxData[3] << CAN_TDL0R_DATA3_Pos) |
      ((uint32_t)TxData[2] << CAN_TDL0R_DATA2_Pos) |
      ((uint32_t)TxData[1] << CAN_TDL0R_DATA1_Pos) |
      ((uint32_t)TxData[0] << CAN_TDL0R_DATA0_Pos));

    // 送信を要求する
    SET_BIT(CAN->sTxMailBox[transmitmailbox].TIR, CAN_TI0R_TXRQ);

    Serial.println("----------------------------------------------------------");
    Serial.println(CAN->sTxMailBox[transmitmailbox].TDHR, BIN);
    Serial.println(CAN->sTxMailBox[transmitmailbox].TDLR, BIN);

    while (CAN_GetTxMailboxesFreeLevel(CAN) != 3);
  }
}


void CANBUS::initializeMsp() {
  // CANのクロックを有効にする
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_CAN);
  // GPIOAのクロックを有効にする
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

  // GPIO Init
  LL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };
  GPIO_InitStruct.Pin = LL_GPIO_PIN_11 | LL_GPIO_PIN_12; // Pin選択
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE; // alternate function mode
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH; // SPEED FREQUENT HIGH
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL; // OUTPUT PUSH PULL
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO; // not PULL
  GPIO_InitStruct.Alternate = LL_GPIO_AF_9; // CAN Alternate Function
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct); // 初期化する

  // CANの割り込みを設定する
  // RX0の割り込みを有効にする
  NVIC_SetPriority(CAN_RX0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  NVIC_EnableIRQ(CAN_RX0_IRQn);

  // RX1の割り込みを有効にする
  NVIC_SetPriority(CAN_RX1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  NVIC_EnableIRQ(CAN_RX1_IRQn);
}


void CANBUS::initializeParameters() {
  // BRP[9:0] プリスケーラ 1~1024(BTR)
  uint32_t prescaler = 4;
  // SILM LBKM モードを設定(BTR)
  uint32_t mode = (uint32_t)(0x00 << CAN_BTR_LBKM_Pos); // 通常動作、ループバックモードは無効
  // SJW[1:0] 再同期ジャンプ幅(BTR)
  uint32_t syncJumpWidth = (uint32_t)(0x00 << CAN_BTR_SJW_Pos); // t_RJW =t_q*(SJW[1:0] + 1)より1
  // TS1[3:0] 時間セグメント 1(BTR)
  uint32_t timeSeg1 = (uint32_t)(0x0000000A << CAN_BTR_TS1_Pos); // 11に設定(0のとき1のため11-1=10)
  // TS2[2:0] 時間セグメント 2(BTR)
  uint32_t timeSeg2 = (uint32_t)(0x00000003 << CAN_BTR_TS2_Pos); // 4に設定(0のとき1のため4-1=3)
  // TTCM タイムトリガ通信モード(MCR)
  FunctionalState timeTriggerMode = DISABLE;// 無効に設定
  // ABOM 自動バスオフ管理(MCR)
  FunctionalState autoBussOff = DISABLE; // 無効に設定
  // AWUM 自動ウェイクアップモード(MCR)
  FunctionalState autoWakeUp = DISABLE; // 無効に設定
  // NART 自動再送信禁止(MCR)
  FunctionalState autoRetransmission = DISABLE; // 無効に設定
  // RFLM 受信 FIFO ロックモード(MCR)
  FunctionalState receiveFifoLocked = DISABLE; // 無効に設定
  // TXFP 送信 FIFO 優先順位(MCR)
  FunctionalState transmitFifoPriority = DISABLE; // 無効に設定

  // SLEEPモードから起動させる
  CLEAR_BIT(CAN->MCR, CAN_MCR_SLEEP);
  // SLEEPから起動するまでまつ
  while ((CAN->MSR & CAN_MSR_SLAK) != 0);
  // CANの初期化リクエストを行う
  SET_BIT(CAN->MCR, CAN_MCR_INRQ);
  // 初期化のリクエストの確認応答がくるまでまつ
  while ((CAN->MSR & CAN_MSR_INAK) == 0);
  // ビットを一つずつ設定する
  // タイムトリガ通信モードの設定
  if (timeTriggerMode == ENABLE) SET_BIT(CAN->MCR, CAN_MCR_TTCM);
  else CLEAR_BIT(CAN->MCR, CAN_MCR_TTCM);
  // 自動バスオフ管理の設定
  if (autoBussOff == ENABLE) SET_BIT(CAN->MCR, CAN_MCR_ABOM);
  else CLEAR_BIT(CAN->MCR, CAN_MCR_ABOM);
  // 自動ウェイクアップモードの設定
  if (autoWakeUp == ENABLE) SET_BIT(CAN->MCR, CAN_MCR_AWUM);
  else CLEAR_BIT(CAN->MCR, CAN_MCR_AWUM);
  // 自動再送信禁止の設定
  if (autoRetransmission == ENABLE) CLEAR_BIT(CAN->MCR, CAN_MCR_NART);
  else SET_BIT(CAN->MCR, CAN_MCR_NART);
  // 受信 FIFO ロックモードの設定
  if (receiveFifoLocked == ENABLE) SET_BIT(CAN->MCR, CAN_MCR_RFLM);
  else CLEAR_BIT(CAN->MCR, CAN_MCR_RFLM);
  // 送信 FIFO 優先順位の設定
  if (transmitFifoPriority == ENABLE) SET_BIT(CAN->MCR, CAN_MCR_TXFP);
  else CLEAR_BIT(CAN->MCR, CAN_MCR_TXFP);
  // BTRレジスタの設定を行う
  WRITE_REG(CAN->BTR, mode | syncJumpWidth | timeSeg1 | timeSeg2 | (prescaler - 1U));
}


void CANBUS::initializeFilterParameters() {
  // フィルタバンクの設定(0~13)
  uint32_t filterBank = 0;
  // CAN_filter_modeの設定を行う
  uint8_t filterMode = CAN_FILTER_MODE_IDLIST;
  // CAN_filter_scaleの設定(16bit or 32bit)
  uint8_t filterScale = CAN_FILTER_SCALE_16BIT;
  // フィルタの識別番号Highを設定(MSBs)
  uint32_t filterIdHigh = 0x123 << 5; // or 0x456 << 5
  // フィルタの識別番号Lowを設定(LSBs)
  uint32_t filterIdLow = 0x123 << 5; // or 0x456 << 5
  // フィルタの識別番号Highのマスクを設定(MSBs)
  uint32_t filterMaskIdHigh = 0x123 << 5; // or 0x456 << 5
  // フィルタの識別番号Lowのマスクを設定(LSBs)
  uint32_t filterMaskIdLow = 0x123 << 5; // or 0x456 << 5
  // CAN_filter_FIFOの設定を行う
  uint8_t filterFIFOAssignment = CAN_FILTER_FIFO0;
  // filterの有効化or無効化を設定
  FunctionalState filterActivation = ENABLE;

  uint32_t filternbrbitpos;
  // フィルターの初期化モードにする
  SET_BIT(CAN->FMR, CAN_FMR_FINIT);
  // 使用するフィルタを一度無効にする
  filternbrbitpos = (uint32_t)1 << (filterBank & 0x1FU);
  CLEAR_BIT(CAN->FA1R, filternbrbitpos);
  // フィルターのスケールが16bitのときと32bitのときで設定方法を変える
  if (filterScale == CAN_FILTER_SCALE_16BIT) {
    // フィルターのスケール設定を行う
    CLEAR_BIT(CAN->FS1R, filternbrbitpos);
    // CAN フィルタバンク i レジスタ x
    // CAN_FIR[2:1]から構成されているのでそれぞれ設定する必要がある。
    CAN->sFilterRegister[filterBank].FR1 =
      ((0x0000FFFFU & filterMaskIdLow) << 16U) |
      (0x0000FFFFU & filterIdLow);
    CAN->sFilterRegister[filterBank].FR2 =
      ((0x0000FFFFU & filterMaskIdHigh) << 16U) |
      (0x0000FFFFU & filterIdHigh);
  }

  if (filterScale == CAN_FILTER_SCALE_32BIT) {
    // フィルターのスケール設定を行う
    SET_BIT(CAN->FS1R, filternbrbitpos);
    // CAN フィルタバンク i レジスタ x
    // CAN_FIR[2:1]から構成されているのでそれぞれ設定する必要がある。
    CAN->sFilterRegister[filterBank].FR1 =
      ((0x0000FFFFU & filterIdHigh) << 16U) |
      (0x0000FFFFU & filterIdLow);

    CAN->sFilterRegister[filterBank].FR2 =
      ((0x0000FFFFU & filterMaskIdHigh) << 16U) |
      (0x0000FFFFU & filterMaskIdLow);
  }

  // filterのモードに対しての設定
  if (filterMode == CAN_FILTER_MODE_IDMASK) {
    // 識別子マスクモード
    CLEAR_BIT(CAN->FM1R, filternbrbitpos);
  }
  else {
    // 識別子リストモード
    SET_BIT(CAN->FM1R, filternbrbitpos);
  }

  // FIFOの設定
  if (filterFIFOAssignment == CAN_FILTER_FIFO0) {
    // フィルタが FIFO 0 に割り当てられる
    CLEAR_BIT(CAN->FFA1R, filternbrbitpos);
  }
  else {
    // フィルタが FIFO 1 に割り当てられる
    SET_BIT(CAN->FFA1R, filternbrbitpos);
  }

  // FILTERを有効にするかどうか
  if (filterActivation == ENABLE) {
    // 最初に無効にしたものを、有効にし直す
    SET_BIT(CAN->FA1R, filternbrbitpos);
  }

  // 初期化モードから通常モードに変更する
  CLEAR_BIT(CAN->FMR, CAN_FMR_FINIT);
}


void CANBUS::start() {
  CLEAR_BIT(CAN->MCR, CAN_MCR_INRQ);
  while ((CAN->MSR & CAN_MSR_INAK) != 0);
}


uint32_t CANBUS::CAN_GetTxMailboxesFreeLevel(CAN_TypeDef* CANx) {
  uint32_t freelevel = 0;
  if ((CANx->TSR & CAN_TSR_TME0) != 0U) freelevel++;
  if ((CANx->TSR & CAN_TSR_TME1) != 0U) freelevel++;
  if ((CANx->TSR & CAN_TSR_TME2) != 0U) freelevel++;

  return freelevel;
}


void CAN_RX0_IRQHandler() {

}


void CAN_RX1_IRQHandler() {

}