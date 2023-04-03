#include"IcsHardSerialClass.h"

const byte EN_PIN = 2;
const long BAUNDRATE = 115200;
const int TIMEOUT = 1000;
IcsHardSerialClass B3M(&Serial, EN_PIN, BAUNDRATE,TIMEOUT);

void setup()
{
    B3M.begin();                        //B3Mと通信開始
    Serial.begin(115200, SERIAL_8N1);   //通信速度、パリティなしに設定
    Serial.begin(115200);
}

void loop()
{


}

int B3MwriteCommand(byte id, byte TxData, byte Address)
{
    byte TxCommand[8];
    byte RxCommand[5];
    unsigned int reData;
    bool flag;

    TxCommand[0] = (byte)(0x08);        //SIZE      //一連コマンドのバイト数。今回は8バイト
    TxCommand[1] = (byte)(0x04);        //COMMAND   //何をするための処理か設定。 0x04はWrite
    TxCommand[2] = (byte)(0x00);        //OPTION    //ステータスの読み取り。 0x00はERROR STATUS
    TxCommand[3] = (byte)(id);          //ID        //制御するサーボID番号を指定

    TxCommand[4] = (byte)(TxData);      //DATA      // 
    TxCommand[5] = (byte)(Address);     //ADDRESS   //

    TxCommand[6] = (byte)(0x01);        //COUNT
    TxCommand[7] = (byte)(0x00);        //初期化
 
    for(int i = 0; i < 7; i++)
    {
        TxCommand[7] += TxCommand[i];
        Serial.println(TxCommand[i]);
    }
    TxCommand[7] = (byte)(TxCommand[7]); //CHECKSUM

    flag = B3M.synchronize(TxCommand, sizeof TxCommand, RxCommand, sizeof RxCommand);
    if (flag == false)
    {
        return -1;
    }
    reData = RxCommand[2];

    return reData;
}

int B3MsetPosition(byte id, int Pos, int Time)
{
    byte TxCommand[9];
    byte RxCommand[7];
    unsigned int reData;
    bool flag;

    TxCommand[0] = (byte)(0x09);                //SIZE
    TxCommand[1] = (byte)(0x06);                //COMMAND //0x06はポジションを変更する
    TxCommand[2] = (byte)(0x00);                //OPTION
    TxCommand[3] = (byte)(id);                  //ID

    TxCommand[4] = (byte)(Pos & 0xFF);          //POS_L
    TxCommand[5] = (byte)(Pos >> 8 & 0xFF);     //POS_H

    TxCommand[6] = (byte)(Time & 0xFF);         //TIME_L
    TxCommand[7] = (byte)(Time >> 8 & 0xFF);    //TIME_H

    TxCommand[8] = 0x00;

    for(int i = 0; i < 8; i++)
    {
        TxCommand[8] += TxCommand[i];
    }
    TxCommand[8] = (byte)(TxCommand[8]);        //SUM

    flag = B3M.synchronize(TxCommand, sizeof TxCommand, RxCommand, sizeof RxCommand);
    if (flag == false)
    {
        return -1;
    }
    reData = RxCommand[2];

    return reData;
}