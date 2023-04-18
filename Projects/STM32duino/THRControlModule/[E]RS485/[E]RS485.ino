/******************************************************************/
/*  動作確認中環境                                                */
/*    開発環境  ：VS code [Arduino]                               */
/*    ボード    ：Nucleo-64 F401RE                                */
/******************************************************************/

/* グローバル変数定義 */
// int REDE = PF1; // デジタルPin5(D5)を送信イネーブルピンに設定
int REDE = 6;
// メモ
// HardwereSerial(RX, TX);
// HardwareSerial RS485(PA_10, PA_9);
constexpr int POSITION_CHANGING_THRESHOLD = 10;
int LCount = 0;
int WCount = 0;

int Position = 1;

/*-------------------------------------------------*/
/* 機能   : STM32duino 初期化                         */
/* 名前   : setup                                  */
/* 引数   : 無し                                   */
/* 戻り値 : 無し                                   */
/*-------------------------------------------------*/
void setup()
{
    pinMode(REDE, OUTPUT);        // デジタルPin5(REDE)を出力に設定
    pinMode(LED_BUILTIN, OUTPUT); // デバック用LEDを出力に設定
    Serial2.begin(115200);        // ボーレート 115,200bps

    Torque(0x01, 0x01);

    pinMode(8, INPUT_PULLUP);
    pinMode(9, INPUT_PULLUP);
}

/*-------------------------------------------------*/
/* 機能   : サーボトルク設定                       */
/* 名前   : Torque                                 */
/* 引数   : ID (Servo ID)                          */
/*        : data (Torque enable)                   */
/* 戻り値 : 無し                                   */
/*-------------------------------------------------*/
void Torque(unsigned char ID, unsigned char data)
{
    unsigned char TxData[9];    // 送信データバッファ [9byte]
    unsigned char CheckSum = 0; // チェックサム計算用変数

    // パケットデータ生成
    TxData[0] = 0xFA; // Header
    TxData[1] = 0xAF; // Header
    TxData[2] = ID;   // ID
    TxData[3] = 0x00; // Flags
    TxData[4] = 0x24; // Address
    TxData[5] = 0x01; // Length
    TxData[6] = 0x01; // Count
    TxData[7] = data; // Data
    // チェックサム計算
    for (int i = 2; i <= 7; i++)
    {
        CheckSum = CheckSum ^ TxData[i]; // ID～DATAまでのXOR
    }
    TxData[8] = CheckSum; // Sum

    // パケットデータ送信
    digitalWrite(REDE, HIGH); // 送信許可
    for (int i = 0; i <= 8; i++)
    {
        Serial2.write(TxData[i]);

        // メモ
        // RS485.write(TxData[i]);
    }
    Serial2.flush(); // データ送信完了待ち

    // メモ
    // RS485.flush();
    // delayMicroseconds(700); //必要ないかも
    digitalWrite(REDE, LOW); // 送信禁止
}

/*-------------------------------------------------*/
/* 機能   : サーボ角度・速度指定                   */
/* 名前   : Move                                   */
/* 引数   : ID (Servo ID)                          */
/*        : Angle (Present Posion L&H)             */
/*        : Speed (Present Time L&H)               */
/* 戻り値 : 無し                                   */
/*-------------------------------------------------*/
void Move(unsigned char ID, int Angle, int Speed)
{
    unsigned char TxData[12];   // 送信データバッファ [12byte]
    unsigned char CheckSum = 0; // チェックサム計算用変数

    // パケットデータ生成
    TxData[0] = 0xFA; // Header
    TxData[1] = 0xAF; // Header
    TxData[2] = ID;   // ID
    TxData[3] = 0x00; // Flags
    TxData[4] = 0x1E; // Address
    TxData[5] = 0x04; // Length
    TxData[6] = 0x01; // Count
    // Angle
    TxData[7] = (unsigned char)0x00FF & Angle;        // Low byte
    TxData[8] = (unsigned char)0x00FF & (Angle >> 8); // Hi  byte
    // Speed
    TxData[9] = (unsigned char)0x00FF & Speed;         // Low byte
    TxData[10] = (unsigned char)0x00FF & (Speed >> 8); // Hi  byte
    // チェックサム計算
    for (int i = 2; i <= 10; i++)
    {
        CheckSum = CheckSum ^ TxData[i]; // ID～DATAまでのXOR
    }
    TxData[11] = CheckSum; // Sum

    // パケットデータ送信
    digitalWrite(REDE, HIGH); // 送信許可
    for (int i = 0; i <= 11; i++)
    {
        Serial2.write(TxData[i]);

        // メモ
        // RS485.write(TxData[i]);
    }
    Serial2.flush(); // データ送信完了待ち

    // メモ
    // RS485.flush();
    // delayMicroseconds(700); //必要ないかも
    digitalWrite(REDE, LOW); // 送信禁止
}

/*-------------------------------------------------*/
/* 機能   : メインプログラム                       */
/* 名前   : loop                                   */
/* 引数   : 無し                                   */
/* 戻り値 : 無し                                   */
/*-------------------------------------------------*/
void loop()
{

    // setup();    // STM32duino 初期化
    //delay(100); // wait (100msec)
    Torque(0x01, 0x01); // ID = 1(0x01) , torque = ON   (0x01)
                        // torque = OFF(0x00), ON(0x01), BRAKE(0x02)
    //delay(100); // wait (100msec)

     Move(1, 900, 50);
     delay(1000);
     Move(1, 0, 50);

    // WaitingポジションかつLaunch信号がHIGHならLCountを加算する。それ以外ならLCountを0にリセットする
    // if (Position == 1 && digitalRead(8) == LOW)
    // {
    //     LCount++;
    // }
    // else
    // {
    //     LCount = 0;
    // }

    // // LCountが閾値以上になればLaunchポジションに変更する
    // if (LCount >= POSITION_CHANGING_THRESHOLD)
    // {
    //     LCount = 0;
    //     Move(1, 900, 50);
    //     Position = 2;
    //     digitalWrite(LED_BUILTIN, 0x01);
    //     delay(100);
    // }

    // // 以下、WaitingとLaunchが逆になったバージョン
    // if (Position == 2 && digitalRead(9) == LOW)
    // {
    //     WCount++;
    // }
    // else
    // {
    //     WCount = 0;
    // }
    // if (WCount >= POSITION_CHANGING_THRESHOLD)
    // {
    //     WCount = 0;

    //     Move(1, 0, 50);
    //     Position = 1;
    //     digitalWrite(LED_BUILTIN, 0x00);
    //     delay(100);
    // }
}