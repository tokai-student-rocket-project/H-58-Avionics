//
//  @file B3M_ArduinoNanoEvery_setPos_Sample.ino
//  @brief B3M_ArduinoNanoEvery_setPos_Sample
//  @author Kondo Kagaku Co.,Ltd.
//  @date 2020/4/6
//
//  B3MサーボをArduino Nano Everyから制御
//  通信はICSライブラリのsynchronizeを利用する
//
//  EveryのUART1で通信

#include "IcsHardSerialClass.h"
// #include "IcsBaseClass.h"
// #include "HardwareSerial.h"

const byte EN_PIN = 3;
const long BAUDRATE = 115200;
const int TIMEOUT = 100; // 通信できてないか確認用にわざと遅めに設定

// HardwareSerial Serial1(PA_3, PA_2);
// HardwareSerial Serial1(PA_10, PA_9);
// HardwareSerial Serial1(PB_7, PB_6);

// #define PIN_SERIAL1_RX PA9
// #define PIN_SERIAL1_TX PA10

// IcsHardSerialClass B3M(&Serial1, EN_PIN, BAUDRATE, TIMEOUT); // インスタンス＋ENピン(2番ピン)およびUARTの指定
IcsHardSerialClass B3M(&Serial, EN_PIN, BAUDRATE, TIMEOUT); // インスタンス＋ENピン(2番ピン)およびUARTの指定

void setup()
{
  // Serial1.setRtsCts(PA_12, PA_11);

  // Serial1.setRx(PA_10);
  // Serial1.setTx(PA_9);
  Serial.begin(115200, SERIAL_8E1); // パリティをなしに設定
  // pinMode(PA_10, INPUT_PULLUP);

  // Serial1.begin(115200);
  // Serial.begin(115200);

  B3M.begin();
  // Writeコマンドで設定
  // B3M_WriteCmd(int id, int Data, int Address)
  B3M_WriteCmd(0x00, 0x02, 0x28); // 動作モード：Free
  delay(500);

  B3M_WriteCmd(0x00, 0x02, 0x28); // 位置制御モードに設定
  delay(500);

  B3M_WriteCmd(0x00, 0x01, 0x29); // 起動生成タイプ：Even
  delay(500);

  B3M_WriteCmd(0x00, 0x00, 0x5C); // ゲインプリセット：No.0
  delay(500);

  B3M_WriteCmd(0x00, 0x00, 0x28); // 動作モード：Nomal
  delay(500);
}

void loop()
{

  // Positionコマンドで動作角指定
  // B3M_setPos(int id, int Pos, int Time)
  delay(100);
  B3M_setPos(0x00, 5000, 500);
  delay(1000);
  //Serial.println(reData);

  B3M_setPos(0x00, -5000, 1000);
  delay(1000);
}

// Writeコマンド（）データが1つだけの時）
int B3M_WriteCmd(byte id, byte TxData, byte Address)
{

  byte txCmd[8];
  byte rxCmd[5];
  unsigned int reData;
  bool flg;

  txCmd[0] = (byte)(0x08);    // SIZE
  txCmd[1] = (byte)(0x04);    // CMD
  txCmd[2] = (byte)(0x00);    // OP
  txCmd[3] = (byte)(id);      // ID
  txCmd[4] = (byte)(TxData);  // DATA
  txCmd[5] = (byte)(Address); // ADR
  txCmd[6] = (byte)(0x01);    // CNT
  txCmd[7] = (byte)(0x00);    // SUM(初期化)

  // チェックサムを計算
  for (int i = 0; i < 7; i++)
  {
    txCmd[7] += txCmd[i];
  }
  txCmd[7] = (byte)(txCmd[7]); // SUM

  // コマンドを送受信
  flg = B3M.synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);

  if (flg == false) // もし通信エラーが起きたら-1を返し、処理終了
  {
    return -1;
  }

  reData = rxCmd[2]; // 正常であればステータスを返し、処理終了
  return reData;
}

int B3M_setPos(byte id, int Pos, int Time)
{

  byte txCmd[9];
  byte rxCmd[7];
  unsigned int reData;
  bool flg;

  txCmd[0] = (byte)(0x09); // SIZE
  txCmd[1] = (byte)(0x06); // CMD
  txCmd[2] = (byte)(0x00); // OP
  txCmd[3] = (byte)(id);   // ID

  // ポジションを2バイトに分割
  txCmd[4] = (byte)(Pos & 0xFF);      // POS_L
  txCmd[5] = (byte)(Pos >> 8 & 0xFF); // POS_H

  // タイムを2バイトに分割
  txCmd[6] = (byte)(Time & 0xFF);      // TIME_L
  txCmd[7] = (byte)(Time >> 8 & 0xFF); // TIME_H

  // チェックサムを計算
  txCmd[8] = 0x00; // SUM(初期化)
  for (int i = 0; i < 8; i++)
  {
    txCmd[8] += txCmd[i];
  }
  txCmd[8] = (byte)(txCmd[8]); // SUM

  // コマンドを送受信
  flg = B3M.synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);

  if (flg == false) // もし通信エラーが起きたら-1を返し、処理終了
  {
    return -1;
  }

  reData = rxCmd[2]; // 正常であればステータスを返し、処理終了
  return reData;
}