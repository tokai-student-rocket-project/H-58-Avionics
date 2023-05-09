// #include "IcsBaseClass.h"
#include "IcsHardSerialClass.h"

// #include "SHIN_IcsHardSerialClass.h"

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

const byte EN_PIN = 3;
const long BAUDRATE = 115200;
const int TIMEOUT = 1000;

// HardwareSerial SerialX(RX, TX); //メモ
// RXD(データ受信に使用)
// TXD(データ送信に使用)

// HardwareSerial Serial1(PA_10, PA_9); //STM32F303K8

IcsHardSerialClass B3M(&Serial1, EN_PIN, BAUDRATE, TIMEOUT); // インスタンス＋ENピン(2番ピン)およびUARTの指定
// IcsHardSerialClass B3M(&Serial2, EN_PIN, BAUDRATE, TIMEOUT); // インスタンス＋ENピン(2番ピン)およびUARTの指定

void setup()
{
  B3M.begin();

  // Serial1.setRtsCts(PA_12, PA_11); //特に影響なし
  // Serial1.setRx(PA_10); //STM32F303K8
  // Serial1.setTx(PA_9); //STM32F303K8

  Serial1.begin(115200, SERIAL_8N1); // SERIAL_8N1 >> パリティをなしに設定。
  // Serial2.begin(115200, SERIAL_8N1); // SERIAL_8N1 >> パリティをなしに設定。

  Serial.begin(115200); // デバック用にSerialを設定。PCと接続。

  // Serial2.setRx(PA_3); //STM32coreに書いてあった。多分いらない。
  // Serial2.setTx(PA_2);　//STM32coreに書いてあった。多分いらない。
  // Serial2.begin(BAUDRATE, SERIAL_8N1); // パリティをなしに設定。

  // Serial1.begin(115200);
  // Serial.begin(115200, SERIAL_8N1);

  delay(100);
  pinMode(13, OUTPUT); // 13番のLEDを設定。

  // B3M_ResetCmd(0x80, 0x00); // RESETコマンド

  // Writeコマンドで設定
  // B3M_WriteCmd(int id, int Data, int Address)

  // int B3M_WriteCmd_1;
  B3M_WriteCmd(0x01, 0x02, 0x28);

  // B3M_WriteCmd_1 = B3M_WriteCmd(0x01, 0x02, 0x28); // 動作モード：Free
  // Serial.print("Mode:Free = ");
  // Serial.println(B3M_WriteCmd_1);
  // Serial.println("");

  LED_blink();
  delay(500);

  // int B3M_WriteCmd_2;
  B3M_WriteCmd(0x01, 0x02, 0x28); // 位置制御モードに設定

  // B3M_WriteCmd_2 = B3M_WriteCmd(0x01, 0x02, 0x28); // 位置制御モードに設定
  // Serial.print("Mode:Point = ");
  // Serial.println(B3M_WriteCmd_2);

  LED_blink();
  delay(500);

  // int B3M_WriteCmd_3;
  B3M_WriteCmd(0x01, 0x01, 0x29); // 起動生成タイプ：Even

  // B3M_WriteCmd_3 = B3M_WriteCmd(0x01, 0x01, 0x29); // 起動生成タイプ：Even
  // Serial.print("Type:EVEN = ");
  // Serial.println(B3M_WriteCmd_3);

  LED_blink();
  delay(500);

  // int B3M_WriteCmd_4;
  B3M_WriteCmd(0x01, 0x00, 0x5C); // ゲインプリセット：No.0

  // B3M_WriteCmd_4 = B3M_WriteCmd(0x01, 0x00, 0x5C); // ゲインプリセット：No.0
  // Serial.print("Gain:No.0 = ");
  // Serial.println(B3M_WriteCmd_4);

  LED_blink();
  delay(500);

  // int B3M_WriteCmd_5;
  B3M_WriteCmd(0x01, 0x00, 0x28); // 動作モード：Normal

  // B3M_WriteCmd_5 = B3M_WriteCmd(0x01, 0x00, 0x28); // 動作モード：Normal
  // Serial.print("Mode:Normal = ");
  // Serial.println(B3M_WriteCmd_5);

  LED_blink();
  delay(500);
}

void loop()
{
  // Positionコマンドで動作角指定
  // B3M_setPos(int id, int Pos, int Time)
  delay(100);
  // B3M_ResetCmd(0x80, 0x00);
  B3M_setPos(0x01, 5000, 500);
  // int B3M_setPos_1;
  // B3M_setPos_1 = B3M_setPos(0x01, 5000, 500);
  // Serial.print("B3M_setPos_1 = ");
  // Serial.println(B3M_setPos_1);

  delay(1000);

  // B3M_ResetCmd(0x80, 0x00);
  B3M_setPos(0x01, -5000, 1000);

  // int B3M_setPos_2;
  // B3M_setPos_2 = B3M_setPos(0x01, -5000, 1000);
  // Serial.print("B3M_setPos_2 = ");
  // Serial.println(B3M_setPos_2);

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
  // txCmd[7] = (byte)(0x00);    // SUM(初期化)
  txCmd[7] = 0x00; // SUM(初期化)

  // チェックサムを計算
  for (int i = 0; i < 7; i++)
  {
    txCmd[7] += txCmd[i];
    // Serial.print(txCmd[i], HEX);
    // Serial.print(" ");
  }
  txCmd[7] = (byte)(txCmd[7]); // SUM
  // Serial.println(txCmd[7], HEX);

  // コマンドを送受信
  flg = B3M.synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);

  Serial.println(flg);

  if (flg == false) // もし通信エラーが起きたら-1を返し、処理終了
  {
    Serial.println("synchronize ERROR");
    return -1;
  }

  reData = rxCmd[5]; // 正常であればステータスを返し、処理終了
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
  // txCmd[8] = 0x00; // SUM(初期化)
  txCmd[8] = (byte)(0x00); 
  for (int i = 0; i < 8; i++)
  {
    txCmd[8] += txCmd[i];
    // Serial.print(txCmd[i], HEX);
    // Serial.print(" ");
  }
  txCmd[8] = (byte)(txCmd[8]); // SUM
  // Serial.println(" ");
  // Serial.println(txCmd[8], HEX);

  // コマンドを送受信
  flg = B3M.synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);

  if (flg == false) // もし通信エラーが起きたら-1を返し、処理終了
  {
    Serial.println("synchronize ERROR");
    return -1;
  }

  reData = rxCmd[7]; // 正常であればステータスを返し、処理終了
  return reData;
}

// void B3M_ResetCmd(byte Option, byte id)
// {
//   byte txCmd[6];
//   byte rxCmd[0];
//   unsigned int reData;
//   bool flg;

//   txCmd[0] = (byte)(0x06);   // SIZE
//   txCmd[1] = (byte)(0x05);   // CMD
//   txCmd[2] = (byte)(Option); // OP
//   txCmd[3] = (byte)(id);     // ID
//   txCmd[4] = (byte)(0x03);   // TIME
//   txCmd[5] = (byte)(0x00);   // SUM(初期化)

//   for (int i = 0; i < 5; i++)
//   {
//     txCmd[5] += txCmd[i];
//     Serial.print(txCmd[i], HEX);
//     Serial.print(" ");
//   }
//   txCmd[7] = (byte)(txCmd[7]); // SUM
//   Serial.println(" ");

//   flg = B3M.synchronize(txCmd, 6, rxCmd, 0);

//   if (flg == false)
//   {
//     Serial.println("synchronize ERROR");
//     return -1;
//   }

//   reData = rxCmd[0]; // 正常であればステータスを返し、処理終了
//   return reData;
// }

void LED_blink()
{
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
}
