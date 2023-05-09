#include "IcsHardSerialClass.h"
// #include "EX_MAX31855.h" //test用のhファイル
// #include "HardwareSerial.h"

/*B3M Servo Config START*/
const byte EN_PIN = 3;
const long BAUNDRATE = 115200;
const int TIMEOUT = 1000;
// HardwareSerial SerialX(RX, TX); X = 1, 2, 3...
// HardwareSerial Serial1(PA_10, PA_9); // STM32F303K8

//Everyくんを搭載する可能性大

IcsHardSerialClass B3M(&Serial1, EN_PIN, BAUNDRATE, TIMEOUT);
/*B3M Servo Config END*/

/*Position Change Config START*/
constexpr int POSITION_CHANGING_THRESHOLD = 10;
int Launch_Count = 0;
int Waiting_Count = 0;
int Position = 1;
/*Position Change Config END*/

/*MAX31855 Config START*/
// int32_t rawData = 0;
// MAX31855 myMAX31855(3);
/*MAX31855 Config END*/

void setup()
{
    pinMode(8, INPUT_PULLUP);
    pinMode(9, INPUT_PULLUP);

    B3M.begin(); // B3Mと通信開始
    pinMode(13, OUTPUT);

    Serial1.begin(115200, SERIAL_8N1); // 通信速度、パリティなしに設定
    // Serial2.begin(115200);
    Serial.begin(115200);

    B3MwriteCommand(0x01, 0x02, 0x28); // 動作モード：Free
    LED_Blink();
    delay(500);
    B3MwriteCommand(0x01, 0x02, 0x28); // 位置制御モードに設定
    LED_Blink();
    delay(500);
    B3MwriteCommand(0x01, 0x01, 0x29); // 起動生成タイプ：Even
    LED_Blink();
    delay(500);
    B3MwriteCommand(0x01, 0x00, 0x5c); // ゲインプリセット：No.0
    LED_Blink();
    delay(500);
    B3MwriteCommand(0x01, 0x00, 0x28); // 動作モード：Normal
    LED_Blink();
    delay(1000);

    B3MsetPosition(0x01, 4500, 100);

    // myMAX31855.begin();
    // while (myMAX31855.getChipID() != MAX31855_ID)
    // {
    //     Serial.println(F("MAX31855 error"));
    //     delay(5000);
    // }

    // Serial.println("");
    // Serial.println(F("MAX31855 GO"));
}

void loop()
{
    // MAX31855Errornotification(); //MAX31855 のエラーをお知らせ

    // rawData = myMAX31855.readRawData();

    // Serial.print(F("ColdJuncction = "));
    // Serial.println(myMAX31855.getColdJunctionTemperature(rawData));

    // Serial.print(F("ThermoCouple = "));
    // Serial.println(myMAX31855.getTemperature(rawData));

    if (Position == 1 && digitalRead(8) == LOW)
    {
        Launch_Count++;
    }
    else
    {
        Launch_Count = 0;
    }

    if (Launch_Count >= POSITION_CHANGING_THRESHOLD)
    {
        Launch_Count = 0;
        B3MsetPosition(0x01, 4500, 100);
        digitalWrite(13, HIGH);
        // Serial.println(Launch_Count);
        Position = 2;
        delay(50);
    }
    // Serial.println(Launch_Count);

    //-------------------------------------------------//

    if (Position == 2 && digitalRead(9) == LOW)
    {
        Waiting_Count++;
    }
    else
    {
        Waiting_Count = 0;
    }

    if (Waiting_Count >= POSITION_CHANGING_THRESHOLD)
    {
        Waiting_Count = 0;
        B3MsetPosition(0x01, -4500, 100);
        digitalWrite(13, LOW);
        // Serial.println(Waiting_Count);
        Position = 1;
        delay(50);
    }
    //  Serial.println(Waiting_Count);

    //---テスト用---//
    // Positionで動作角指定

    // B3MsetPosition(0x01, 9000, 5000);
    // delay(1000);
    // // Serial.println("OPEN");
    // // Serial1.println("Hello");
    // B3MsetPosition(0x01, -9000, 5000);
    // delay(1000);
    // // Serial.println("CLOSE");
    // // Serial1.println("TSRP");
}

int B3MwriteCommand(byte id, byte TxData, byte Address)
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

    for (int i = 0; i < 7; i++)
    {
        txCmd[7] += txCmd[i];
        Serial.println(txCmd[i]);
    }
    txCmd[7] = (byte)(txCmd[7]); // CHECKSUM

    // flag = B3M.synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
    flag = B3M.synchronize(txCmd, 8, rxCmd, 5);

    if (flag == false)
    {
        return -1;
    }
    reData = rxCmd[2];

    return reData;
}

int B3MsetPosition(byte id, int Pos, int Time)
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

    for (int i = 0; i < 8; i++)
    {
        txCmd[8] += txCmd[i];
    }
    txCmd[8] = (byte)(txCmd[8]); // SUM

    // flag = B3M.synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
    flag = B3M.synchronize(txCmd, 9, rxCmd, 7);

    if (flag == false)
    {
        return -1;
    }
    reData = rxCmd[7];
    for (int i = 0; i < 7; i++)
    {
        Serial.print(reData, HEX);
        Serial.print(" ");
    }

    return reData;
}

// int B3MreadPOSITION(byte id, byte RxData, byte Address)
// {
//     byte txCmd[7];
//     byte rxCmd[5];
// }

// void MAX31855Errornotification()
// {
//     while (myMAX31855.detectThermocouple() != MAX31855_THERMOCOUPLE_OK)
//     {
//         switch (myMAX31855.detectThermocouple())
//         {
//         case MAX31855_THERMOCOUPLE_SHORT_TO_VCC:
//             Serial.println(F("Thermocouple short to VCC"));
//             break;

//         case MAX31855_THERMOCOUPLE_SHORT_TO_GND:
//             Serial.println(F("Thermocouple short to GND"));
//             break;

//         case MAX31855_THERMOCOUPLE_NOT_CONNECTED:
//             Serial.println(F("Thermocouple not connected"));
//             break;

//         case MAX31855_THERMOCOUPLE_UNKNOWN:
//             Serial.println(F("Thermocouple unknown error"));
//             break;

//         case MAX31855_THERMOCOUPLE_READ_FAIL:
//             Serial.println(F("Thermocouple read error, check chip & spi cable"));
//             break;
//         }
//         delay(5000);
//     }
// }

void LED_Blink()
{
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
}
