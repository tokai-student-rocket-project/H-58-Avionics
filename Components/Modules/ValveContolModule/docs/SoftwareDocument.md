# ValveControlModule Software Document

## 概要

プログラム初心者が書いています．

ValveControlModuleのソフトウェアについて記しています．

## ヘッダーファイル（hogehoge.h）

`
#include <TaskManager.h>
`

- 行いたい処理（タスク）を任意のタイミングで行うことができるようになります．10Hzで動作させたり，電源投入後3回だけ動作させることができたりととても便利なライブラリです．

`
#include "IcsHardSerialClass.h"
`

- 近藤科学さんが販売しているB3Mサーボモータを制御するために使用しています．サーボモータを動作させるだけでなく，サーボモータのRAMに保存されている情報を読み出すこともできます．

- ""で囲んでいる理由は，Arduino Library Manager からインストールしているのではない為です．VScode for Arduinoの仕様上，Arduino(.ino)ファイルと同じフォルダにヘッダーファイル(.h)やcppファイル(.cpp)を置くことで自作ライブラリのように振舞い，コードの内容を変更したりすることができます．IcsHardSerialClass.hはマニュアル通りにArduino Library Managerからインストールするとエラーが発生したためArduino(.ino)ファイルと同じフォルダにヘッダーファイル(.h)を置きました．

`
#include <SPI.h>
`

- SPI(Serial Peripheral Interface)通信を行うためのヘッダーファイル

`
#include <mcp2515_can.h>
`

- MCP2515のライブラリ．CAN(Control Area Network)通信を簡単に行うことができる．

## 関数

### 59Hzタスク

バルブがLaunchModeなのかWaitingModeなのかCANで状態を送信している．

~~~
Tasks.add("Mode", []()
              {
                  uint8_t can[1];
                  can[0] = static_cast<uint8_t>(StateTransition::ChangeMode);
                  CAN.sendMsgBuf(0x10A, 0, 1, can);
                  // Serial.print(can[0]);
                  // Serial.println(" | ");
              })
        ->startFps(59);
~~~
`uint8_t can[1];` 
> 符号なし8bit整数型(範囲：0~255)で**can**という名前の[1](1byte = 8bit)の配列を作成．

`can[0] = static_cast<uint8_t>(StateTransition::ChangeMode);` 
> 添字0の要素に列挙型(enum)から符号なし8bit整数型(uint8_t)へ明示的型変換を行った値を代入している．
> 
> [ゼロから学ぶC++](https://rinatz.github.io/cpp-book/ch08-01-cpp-casts/)に分かりやすく？掲載されている．

`CAN.sendMsgBuf(0x10A, 0, 1, can);`
> mcp_2515.h のライブラリ内に定義してある関数．
>
> 
> |第1引数|第２引数|第3引数|第4引数|
> | :-: | :-: | :-: | :-: |
> | ID | [0]標準フォーマット,[1] 拡張フォーマット | 送信データ長　 (Byte) | Buffer (配列)　

`->startFps(59);`
> TaskManager.hの機能．startFps()<-引数の値のHzで処理を行う．この場合は，59Hz

## void Toque(unsigned char ID, unsigned char data)

戻り値無し．

第1引数 サーボID / 第2引数 送信データ

サーボモータ(RS405CB)のトルクON/OFFを行う．

~~~
void Torque(unsigned char ID, unsigned char data)
{
    unsigned char TxData[9];
    unsigned char CheckSum = 0;

    // パケットデータ生成
    TxData[0] = 0xFA; // HEADER
    TxData[1] = 0xAF; // HEADER
    TxData[2] = ID;   // ID
    TxData[3] = 0x00; // FLAGS
    TxData[4] = 0x24; // ADDRESS
    TxData[5] = 0x01; // LENGTH
    TxData[6] = 0x01; // COUNT
    TxData[7] = data; // DATA

    for (int i = 2; i <= 7; i++)
    {
        CheckSum = CheckSum ^ TxData[i];
    }

    TxData[8] = CheckSum;

    digitalWrite(REDE_PIN, HIGH);
    for (int i = 0; i <= 8; i++)
    {
        Serial1.write(TxData[i]);
    }

    Serial1.flush();
    digitalWrite(REDE_PIN, LOW);
}
~~~

`Torque(0x01, 0x01);`
> 第1引数　0x01(0xで16進数表記)つまり，ID1のサーボモータのトルクを第2引数 0x01 でONにしている．トルクONにしないとサーボモータは動作しない．第2引数で0x00を書き込めばトルクOFFにできる．

## void Move(unsigned char ID, int Angle, int Speed)

戻り値無し．

第1引数 サーボID，第2引数 目標角度，第3引数 目標速度

サーボモータ(RS405CB)を目標の角度，速度で動作させる．

~~~
void Move(unsigned char ID, int Angle, int Speed)
{
    unsigned char TxData[12];
    unsigned char CheckSum = 0;

    // パケットデータ生成
    TxData[0] = 0xFA; // HEADER
    TxData[1] = 0xAF; // HEADER
    TxData[2] = ID;   // ID
    TxData[3] = 0x00; // FLAGS
    TxData[4] = 0x1E; // ADDRESS
    TxData[5] = 0x04; // LENGTH
    TxData[6] = 0x01; // COUNT
    // Angle
    TxData[7] = (unsigned char)0x00FF & Angle;
    TxData[8] = (unsigned char)0x00FF & (Angle >> 8);
    // Speed
    TxData[9] = (unsigned char)0x00FF & Speed;
    TxData[10] = (unsigned char)0x00FF & (Speed >> 8);
    // チェックサム計算
    for (int i = 2; i <= 10; i++)
    {
        CheckSum = CheckSum ^ TxData[i];
    }
    TxData[11] = CheckSum;

    // パケットデータ送信
    digitalWrite(REDE_PIN, HIGH);
    for (int i = 0; i <= 11; i++)
    {
        Serial1.write(TxData[i]);
    }
    Serial1.flush();

    digitalWrite(REDE_PIN, LOW);
}
~~~

`Move(1, 900, 100)`
> 第1引数 ID1 のサーボモータを第2引数 900 = 90.0deg(x 0.1)に設定し，第3引数 100 = 1.0sec(x 0.01)の時間で動かす．

## int B3M_writeCommand(byte id, byte TxData, byte Address)

戻り値 int型．

第1引数 ID, 第2引数 TxDate, 第3引数 Address

サーボモータ(B3M)へ設定を書き込む関数です．
~~~
int B3M_writeCommand(byte id, byte TxData, byte Address)
{
    byte txCmd[8];
    byte rxCmd[5];
    unsigned int reData;
    bool flag;

    txCmd[0] = (byte)(0x08); // SIZE      //一連コマンドのバイト数。今回は8バイト
    txCmd[1] = (byte)(0x04); // COMMAND   //何をするための処理か設定。 0x04はWrite
    txCmd[2] = (byte)(0x00); // OPTION    //ステータスの読み取り。 0x00はERROR STATUS
    txCmd[3] = (byte)(id);   // ID        //制御するサーボID番号を指定

    txCmd[4] = (byte)(TxData);  // DATA      //
    txCmd[5] = (byte)(Address); // ADDRESS   //

    txCmd[6] = (byte)(0x01); // COUNT
    txCmd[7] = (byte)(0x00); // 初期化

    // Serial.print("tx: ");
    for (int i = 0; i < 7; i++)
    {
        txCmd[7] += txCmd[i];
        // Serial.print(txCmd[i]);
        // Serial.print("|");
    }
    // Serial.println("");
    txCmd[7] = (byte)(txCmd[7]); // CHECKSUM

    // flag = B3M.synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
    flag = B3M.synchronize(txCmd, 8, rxCmd, 5);

    if (flag == false)
    {
        return -1;
        Serial.println(F("synchronize ERROR"));
    }

    // Serial.print("rx: ");
    for (int o = 0; o < 7; o++)
    {
        rxCmd[7] += rxCmd[o];
        // Serial.print(rxCmd[o]);
        // Serial.print("|");
    }
    // Serial.println("");

    reData = rxCmd[2];
    return reData;
}
~~~

`B3M_writeCommand(0x01, 0x02, 0x28)`
> B3Mの動作モードをFreeに設定する．
>
> 詳細は[B3MSoftwareManua](../../../../Documents/Datasheets/B3M/B3M_SoftwareManual1.2.0.3.pdf
> )を参照のこと．
>

## int B3M_setposition(byte id, int Pos, int Time)

戻り値 int型

第1引数 ID, 第2引数 目標角度, 第3引数 動作時間

~~~
int B3M_setposition(byte id, int Pos, int Time)
{
    byte txCmd[9];
    byte rxCmd[7];
    unsigned int reData;
    bool flag;

    txCmd[0] = (byte)(0x09); // SIZE
    txCmd[1] = (byte)(0x06); // COMMAND //0x06はポジションを変更する
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(Pos & 0xFF);      // POS_L
    txCmd[5] = (byte)(Pos >> 8 & 0xFF); // POS_H

    txCmd[6] = (byte)(Time & 0xFF);      // TIME_L
    txCmd[7] = (byte)(Time >> 8 & 0xFF); // TIME_H

    txCmd[8] = 0x00;

    // Serial.print("tx: ");
    for (int i = 0; i < 8; i++)
    {
        txCmd[8] += txCmd[i];
        // Serial.print(txCmd[i]);
        // Serial.print("|");
    }
    // Serial.println("|");
    txCmd[8] = (byte)(txCmd[8]); // SUM

    // flag = B3M.synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
    flag = B3M.synchronize(txCmd, 9, rxCmd, 7);

    if (flag == false)
    {
        return -1;
        Serial.println(F("synchronize ERROR"));
    }

    // Serial.print("rx: ");
    for (int o = 0; o < 7; o++)
    {
        // Serial.print(rxCmd[o]);
        // Serial.print("|");
    }
    // Serial.println("");

    reData = rxCmd[7];
    return reData;
}
~~~

`int B3M_setposition(0x01, 9000, 1000)`
> 第1引数 ID1 のサーボモータを第2引数 9000 = 90.0deg(x 0.01)に設定し，第3引数 1000 = 1.0sec(x 0.001)で動作させる．

## void B3M_initialize()

サーボモータ(B3M)を制御できるように設定値を書き込む．

~~~
void B3M_initialize()
{
    B3M_writeCommand(0x01, 0x02, 0x28); // 動作モード：Free

    B3M_writeCommand(0x01, 0x02, 0x28); // 位置制御モードに設定

    B3M_writeCommand(0x01, 0x01, 0x29); // 起動生成タイプ：Even

    B3M_writeCommand(0x01, 0x00, 0x5c); // ゲインプリセット：No.0

    B3M_writeCommand(0x01, 0x00, 0x28); // 動作モード：Normal
}
~~~

`B3M_writeCommand(0x01, 0x02, 0x28);`

`B3M_writeCommand(0x01, 0x02, 0x28);`

`B3M_writeCommand(0x01, 0x01, 0x29);`

`B3M_writeCommand(0x01, 0x00, 0x5c);`

`B3M_writeCommand(0x01, 0x00, 0x28);`

>これら4つを書き込んでからでないとサーボモータを制御することができないため注意．今回は，B3M_initialize()という関数にひとまとめにし，setup()内で1度実行している．

## void SignalInitialize()

戻り値無し．

引数無し．

信号を受け取るピンのpinMode()にてINPUT_PULLUPに設定を行う関数．

~~~
void SignalInitialize()
{
    pinMode(LaunchPin, INPUT_PULLUP);
    pinMode(WaitingPin, INPUT_PULLUP);
}
~~~

> 各pinをINPUT_PULLUP，つまりプルアップ抵抗を繋げた時と同じ動作をするように設定している．Arduinoにスイッチを繋げて何かを動作させることと同様の処理を行っている．

## void BuzzerInitialize()


戻り値無し

引数なし．

ブザーに接続しているピンを出力モードに設定している．

~~~
void BuzzerInitialize()
{
    pinMode(BuzzerPin, OUTPUT);
}
~~~

> pinMode()の第2引数をOUTPUTにすることで出力モードとなる．反対にINPUTにすることで入力モードとなる．LEDを点灯させる事と同様の処理を行っている．Lチカできれば理解できる！

