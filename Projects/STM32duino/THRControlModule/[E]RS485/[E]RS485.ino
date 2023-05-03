/******************************************************************/
/*  動作確認中環境                                                */
/*    開発環境  ：VS code [Arduino]                               */
/*    ボード    ：Nucleo-64 F401RE                                */
/******************************************************************/

/* グローバル変数定義 */
int REDE = 2; // デジタルPin2(D2)を送信イネーブルピンに設定
// メモ
// HardwereSerial(RX, TX);
// HardwareSerial Serial1(PA_10, PA_9);

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
    // Serial1.setRx(PA_10);
    // Serial1.setTx(PA_9);
    // Serial1.begin(115200); // ボーレート 115,200bps
    // Serial.begin(115200);
    Serial2.begin(115200);
    // メモ
    // RS485.begin(115200);
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
        // Serial1.write(TxData[i]);
        // Serial.write(TxData[i]);
        Serial2.write(TxData[i]);
        // メモ
        // RS485.write(TxData[i]);
    }
    // Serial1.flush(); // データ送信完了待ち
    Serial2.flush();
    // Serial.flush();
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
        // Serial1.write(TxData[i]);
        // Serial.write(TxData[i]);
        Serial2.write(TxData[i]);

        // メモ
        // RS485.write(TxData[i]);
    }
    // Serial1.flush(); // データ送信完了待ち
    // Serial.flush();
    Serial2.flush();
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
    setup();            // STM32duino 初期化
    delay(100);         // wait (100msec)
    Torque(0x01, 0x01); // ID = 1(0x01) , torque = ON   (0x01)
                        // torque = OFF(0x00), ON(0x01), BRAKE(0x02)
    delay(100);         // wait (100msec)

    Move(1, 0, 50); // ID = 1 , GoalPosition = 0.00deg(0) , Time = 0.5sec(50)
    digitalWrite(LED_BUILTIN, 0x00);
    delay(1100); // wait (1.10sec)

    Move(1, 900, 100); // ID = 1 , GoalPosition = 90.0deg(900) , Time = 1.0sec(100)
    digitalWrite(LED_BUILTIN, 0x01);
    delay(1000); // wait (1.00sec)
}
