int REDE = 10;
HardwareSerial Serial1(PA_10, PA_9);

void setup()
{
    pinMode(REDE, OUTPUT);
    Serial1.setRx(PA_10);
    Serial1.setTx(PA_9);
    Serial1.begin(115200);

    B3M_WriteCmd(0x01, 0x02, 0x28); // 動作モード：Free
    delay(500);

    B3M_WriteCmd(0x01, 0x02, 0x28); // 位置制御モードに設定
    delay(500);

    B3M_WriteCmd(0x01, 0x01, 0x29); // 起動生成タイプ：Even
    delay(500);

    B3M_WriteCmd(0x01, 0x00, 0x5C); // ゲインプリセット：No.0
    delay(500);

    B3M_WriteCmd(0x01, 0x00, 0x28); // 動作モード：Nomal
    delay(500);
}

void loop()
{
    B3M_setPos(0x01, 5000, 500);
    delay(100);
    //Serial.print("HH");

    B3M_setPos(0x01, -5000, 1000);
    delay(100);
}

void B3M_WriteCmd(byte id, byte TxData, byte Address)
{
    // byte txCmd[8];
    // byte CheckSum = 0;

    byte txCmd[8];
    byte rxCmd[5];
    // unsigned int reData;
    // bool flag;

    txCmd[0] = (byte)(0x08);
    txCmd[1] = (byte)(0x04);
    txCmd[2] = (byte)(0x00);
    txCmd[3] = (byte)(id);
    txCmd[4] = (byte)(TxData);
    txCmd[5] = (byte)(Address);
    txCmd[6] = (byte)(0x01);
    txCmd[7] = (byte)(0x00);

    txCmd[8] = 0x00;
    for (int i = 0; i < 8; i++)
    {
        txCmd[8] += txCmd[i];
        // CheckSum = CheckSum ^ txCmd[i];
    }
    txCmd[8] = (byte)(txCmd[8]);

    digitalWrite(REDE, HIGH);
    for (int i = 0; i <= 8; i++)
    {
        Serial1.write(txCmd[i]);
    }
    Serial1.flush();
    digitalWrite(REDE, LOW);
}

// int B3M_ReadCmd(byte id, byte TxData, byte Address)
// {
//     // byte txCmd[8];
//     // byte CheckSum = 0;

//     byte txCmd[8];
//     byte rxCmd[5];
//     unsigned int reData;
//     // bool flag;

//     txCmd[0] = (byte)(0x08);
//     txCmd[1] = (byte)(0x03); //READ
//     txCmd[2] = (byte)(0x00);
//     txCmd[3] = (byte)(id);
//     txCmd[4] = (byte)(TxData);
//     txCmd[5] = (byte)(Address);
//     txCmd[6] = (byte)(0x01);
//     txCmd[7] = (byte)(0x00);

//     txCmd[8] = 0x00;
//     for (int i = 0; i < 8; i++)
//     {
//         txCmd[8] += txCmd[i];
//         // CheckSum = CheckSum ^ txCmd[i];
//     }
//     txCmd[8] = (byte)(txCmd[8]);

//     digitalWrite(REDE, HIGH);
//     for (int i = 0; i <= 8; i++)
//     {
//         Serial1.Read(txCmd[i]);
        
//     }
//     Serial1.flush();
//     digitalWrite(REDE, LOW);
// }

void B3M_setPos(byte id, int Pos, int Time)
{
    byte txCmd[9];
    byte rxCmd[7];
    // unsigned int reData;
    // bool flg;

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

    digitalWrite(REDE, HIGH);
    for (int i = 0; i <= 8; i++)
    {
        Serial1.write(txCmd[i]);
    }
    Serial1.flush();
    digitalWrite(REDE, LOW);
}
