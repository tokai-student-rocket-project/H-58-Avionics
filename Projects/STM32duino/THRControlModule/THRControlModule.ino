#include "IcsHardSerialClass.h"
#include "EX_MAX31855.h" //test用のhファイル

/*B3M Servo Config START*/
const byte EN_PIN = 2;
const long BAUNDRATE = 115200;
const int TIMEOUT = 1000;
IcsHardSerialClass B3M(&Serial, EN_PIN, BAUNDRATE, TIMEOUT);
/*B3M Servo Config END*/

/*Position Change Config START*/
constexpr int POSITION_CHANGING_THRESHOLD = 10;
int Launch_Count = 0;
int Waiting_Count = 0;
int Position = 1;
/*Position Change Config END*/

/*MAX31855 Config START*/
int32_t rawData = 0;
MAX31855 myMAX31855(3);
/*MAX31855 Config END*/

void setup()
{
    pinMode(8, INPUT_PULLUP);
    pinMode(9, INPUT_PULLUP);

    B3M.begin();                      // B3Mと通信開始
    Serial.begin(115200, SERIAL_8N1); // 通信速度、パリティなしに設定
    Serial.begin(115200);

    B3MwriteCommand(0x00, 0x02, 0x28); // 動作モード：Free
    delay(500);
    B3MwriteCommand(0x00, 0x02, 0x28); // 位置制御モードに設定
    delay(500);
    B3MwriteCommand(0x00, 0x01, 0x29); // 起動生成タイプ：Even
    delay(500);
    B3MwriteCommand(0x00, 0x00, 0x5c); // ゲインプリセット：No.0
    delay(500);
    B3MwriteCommand(0x00, 0x00, 0x28); // 動作モード：Normal
    delay(500);

    myMAX31855.begin();
    while (myMAX31855.getChipID() != MAX31855_ID)
    {
        Serial.println(F("MAX31855 error"));
        delay(5000);
    }

    Serial.println("");
    Serial.println(F("MAX31855 GO"));
}

void loop()
{
    MAX31855Errornotification(); //MAX31855 のエラーをお知らせ

    rawData = myMAX31855.readRawData();

    Serial.print(F("ColdJuncction = "));
    Serial.println(myMAX31855.getColdJunctionTemperature(rawData));

    Serial.print(F("ThermoCouple = "));
    Serial.println(myMAX31855.getTemperature(rawData));

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
        B3MsetPosition(0x00, 5000, 500);
        Position = 2;
        delay(3000);
    }

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
        B3MsetPosition(0x00, -5000, 500);
        Position = 2;
        delay(3000);
    }

    //---テスト用---//
    // Positionで動作角指定
    // B3MsetPosition(int id, int Pos, int Time)
    // B3MsetPosition(0x00, 5000, 500);
    // delay(3000);
    // B3MsetPosition(0x00, -5000, 1000);
    // delay(3000);
}

int B3MwriteCommand(byte id, byte TxData, byte Address)
{
    byte TxCommand[8];
    byte RxCommand[5];
    unsigned int reData;
    bool flag;

    TxCommand[0] = (byte)(0x08); // SIZE      //一連コマンドのバイト数。今回は8バイト
    TxCommand[1] = (byte)(0x04); // COMMAND   //何をするための処理か設定。 0x04はWrite
    TxCommand[2] = (byte)(0x00); // OPTION    //ステータスの読み取り。 0x00はERROR STATUS
    TxCommand[3] = (byte)(id);   // ID        //制御するサーボID番号を指定

    TxCommand[4] = (byte)(TxData);  // DATA      //
    TxCommand[5] = (byte)(Address); // ADDRESS   //

    TxCommand[6] = (byte)(0x01); // COUNT
    TxCommand[7] = (byte)(0x00); // 初期化

    for (int i = 0; i < 7; i++)
    {
        TxCommand[7] += TxCommand[i];
        Serial.println(TxCommand[i]);
    }
    TxCommand[7] = (byte)(TxCommand[7]); // CHECKSUM

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

    TxCommand[0] = (byte)(0x09); // SIZE
    TxCommand[1] = (byte)(0x06); // COMMAND //0x06はポジションを変更する
    TxCommand[2] = (byte)(0x00); // OPTION
    TxCommand[3] = (byte)(id);   // ID

    TxCommand[4] = (byte)(Pos & 0xFF);      // POS_L
    TxCommand[5] = (byte)(Pos >> 8 & 0xFF); // POS_H

    TxCommand[6] = (byte)(Time & 0xFF);      // TIME_L
    TxCommand[7] = (byte)(Time >> 8 & 0xFF); // TIME_H

    TxCommand[8] = 0x00;

    for (int i = 0; i < 8; i++)
    {
        TxCommand[8] += TxCommand[i];
    }
    TxCommand[8] = (byte)(TxCommand[8]); // SUM

    flag = B3M.synchronize(TxCommand, sizeof TxCommand, RxCommand, sizeof RxCommand);
    if (flag == false)
    {
        return -1;
    }
    reData = RxCommand[2];

    return reData;
}

void MAX31855Errornotification()
{
    while (myMAX31855.detectThermocouple() != MAX31855_THERMOCOUPLE_OK)
    {
        switch (myMAX31855.detectThermocouple())
        {
        case MAX31855_THERMOCOUPLE_SHORT_TO_VCC:
            Serial.println(F("Thermocouple short to VCC"));
            break;

        case MAX31855_THERMOCOUPLE_SHORT_TO_GND:
            Serial.println(F("Thermocouple short to GND"));
            break;

        case MAX31855_THERMOCOUPLE_NOT_CONNECTED:
            Serial.println(F("Thermocouple not connected"));
            break;

        case MAX31855_THERMOCOUPLE_UNKNOWN:
            Serial.println(F("Thermocouple unknown error"));
            break;

        case MAX31855_THERMOCOUPLE_READ_FAIL:
            Serial.println(F("Thermocouple read error, check chip & spi cable"));
            break;
        }
        delay(5000);
    }
}