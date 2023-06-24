#include <TaskManager.h>
#include "IcsHardSerialClass.h"
//#include "EX_MAX31855.h" //test用のhファイル
#include "Adafruit_MAX31855.h"
#include <SPI.h>
#include <mcp2515_can.h>

/* VV namespaceをつかう*/

/* CAN Config START */
const int SPI_CS_PIN = 6;
mcp2515_can CAN(SPI_CS_PIN);
// unsigned char sample[8] = {0, 0, 0, 0, 0, 0, 0, 0};
union Converter
{
    // float value, value1;
    double value, value1;
    uint8_t data[4], data1[4];
} converter;

/* CAN Config END */

/*B3M Servo Config START*/
const byte EN_PIN = 2;
const long BAUNDRATE = 115200;
const int TIMEOUT = 500;
IcsHardSerialClass B3M(&Serial1, EN_PIN, BAUNDRATE, TIMEOUT);

/* Memo */
// HardwareSerial SerialX(RX, TX); X = 1, 2, 3...
// HardwareSerial Serial1(PA_10, PA_9); // STM32F303K8
// HardwareSerial Serial1(0, 1); // STM32F303K8
/* Memo */

/*B3M Servo Config END*/

/*RS405CB Servo Config START*/
uint8_t REDE = 3;
int Rs405cbCloseAngle = -800;
int Rs405cbOpenAngle = 0;
/*RS405CB Servo Config END*/

/*Position Change Config START*/
constexpr int POSITION_CHANGING_THRESHOLD = 5;
uint8_t LaunchCount = 0;
uint8_t WaitingCount = 0;
uint8_t Position = 1;
uint8_t WaitingPin = 8;
uint8_t LaunchPin = 7;

/*Position Change Config END*/

/*EX_MAX31855 Config START*/
// int32_t rawData = 0;
// //float temperature;
// MAX31855 myMAX31855(5); // Chip Select PIN (CS)
/*EX_MAX31855 Config END*/

/*MAX31855 Config START*/

// #define MAXCS   10
Adafruit_MAX31855 thermocouple(5);

/*EX_MAX31855 Config END*/

/*MAX31855 Pin Config*/
// MAX31855      Every
// CLK  -------- SCK(D13)
// CS   -------- CS_Pin(D3)
// DO   -------- CIPO(D12)
// GND  -------- GND
// 3Vo  -------- N/C
// Vin  -------- 3~5V

void setup()
{
    // pinMode(A6, OUTPUT); // Busser I/O

    Serial1.begin(115200, SERIAL_8N1); // 通信速度、パリティなしに設定
    Serial1.begin(115200);
    Serial.begin(115200);
    Serial.println("Serial OK");

    /* --- RS405CB Config --- */

    pinMode(REDE, OUTPUT);
    Torque(0x01, 0x01);
    delay(100);
    // Move(1, 900, 100);
    delay(1100);
    Move(1, Rs405cbOpenAngle, 100);

    /* --- RS405CB END Config --- */

    SIGNAL_initialize();
    // MAX31855_initialize();

    /* ---B3M Config--- */

    B3M.begin(); // B3Mと通信開始
    B3M_initialize();
    // B3M_setPosition(0x01, -7500, 1000);
    delay(2000);
    B3M_setPosition(0x01, 0, 1000);

    /* ---B3M Config END--- */

    /* --- CAN --- */

    CAN.begin(CAN_500KBPS, MCP_8MHz); // 500KBPSに固定
    Serial.println("CAN init OK!");

    /* --- CAN Config END --- */

    /* --- MAX31855 ---*/
    thermocouple.begin();
    /* ---  MAX31855 Config END ---*/

    Tasks.add("task", []()
              {
                // Serial.print("Temperature: ");
                // Serial.print(CorrectedTemperature());
                // Serial.print(" | ");
                // Serial.print("ColdJunctionTemperature: ");
                // Serial.print(myMAX31855.getColdJunctionTemperature(rawData));
                // Serial.print(" | ");

                Serial.print("ColdJunctionTemperature: ");
                Serial.print(thermocouple.readInternal());
                Serial.print(" | ");
                Serial.print("Temperature: ");
                Serial.print(thermocouple.readCelsius());
                Serial.print(" | ");
                Serial.print("CorrectedTemperature: ");
                Serial.print(CorrectedTemperature());
                Serial.print(" | ");
                Serial.print("LaunchCount: ");
                Serial.print(LaunchCount);
                Serial.print(" | ");
                Serial.print("WaitingCount: ");
                Serial.println(WaitingCount); })
        ->startFps(100); // 1/10*10^-3 = 100Hz
                         //->startIntervalMsec(1000); // 1/1000*10^-3 = 1Hz
}

void loop()
{
    Tasks.update();

    //rawData = myMAX31855.readRawData();
    
    //myMAX31855.getColdJunctionTemperature(rawData);

    /*↓ここを100Hzで回す*/
    if (Position == 1 && digitalRead(LaunchPin) == LOW)
    {
        LaunchCount++;
    }
    else
    {
        LaunchCount = 0;
    }

    if (LaunchCount >= POSITION_CHANGING_THRESHOLD)
    {
        LaunchCount = 0;
        Serial.println("Launch!!");
        Torque(0x01, 0x01);
        Move(1, -800, 10);                // RS405CBを-80度動作させる //供給と一緒に位置合わせを行った
        delay(200);                       // 200ms 待機
        B3M_setPosition(0x01, -6500, 10); // B3Mを-65度(-6500)動作させる

        Position = 2;
        delay(50);
    }

    // if (digitalRead(LaunchPin) == LOW)
    // {
    //     Torque(0x01, 0x01);
    //     Move(1, -800, 10);                // RS405CBを-80度動作させる //供給と一緒に位置合わせを行った
    //     delay(200);                       // 200ms 待機
    //     B3M_setPosition(0x01, -6500, 10); // B3Mを-65度(-6500)動作させる

    //     Serial.println("Launch!!");
    //     delay(10);
    // }

    //-------------------------------------------------//

    if (Position == 2 && digitalRead(WaitingPin) == LOW)
    {
        WaitingCount++;
    }
    else
    {
        WaitingCount = 0;
    }

    if (WaitingCount >= POSITION_CHANGING_THRESHOLD)
    {
        WaitingCount = 0;
        Serial.println("Waiting!!");
        Move(1, 0, 100);                // RS405CBを0度動作させる //供給と一緒に位置合わせを行った。
        delay(500);                     // 500ms 待機
        B3M_setPosition(0x01, 0, 1000); // B3Mを0度(0000)動作させる

        Position = 1;
        delay(50);
    }

    // if (digitalRead(WaitingPin) == LOW)
    // {
    //     Move(1, 0, 100);                // RS405CBを0度動作させる //供給と一緒に位置合わせを行った。
    //     delay(500);                     // 500ms 待機
    //     B3M_setPosition(0x01, 0, 1000); // B3Mを0度(0000)動作させる

    //     Serial.println("Waiting!!");
    //     delay(10);
    // }

    //MAX31855_errornotification(); // MAX31855 のエラーをお知らせ

    // sample[7] = sample[7] + 1;

    // if (sample[7] == 100)
    // {
    //     sample[7] = 0;
    //     sample[6] = sample[6] + 1;

    //     if (sample[6] == 100)
    //     {
    //         sample[6] = 0;
    //         sample[5] = sample[5] + 1;
    //     }
    // }

    // send data:  id = 0x100, standrad frame, data len = 8, stmp: data buf
    // converter.value = myMAX31855.getTemperature(rawData);
    //converter.value = thermocouple.readCelsius();
    converter.value = CorrectedTemperature();
    CAN.sendMsgBuf(0x100, 0, 4, converter.data);
    // Serial.print("CANmsg: ");
    // Serial.print(converter.data[0]);
    // Serial.print(" | ");
    // Serial.print(converter.data[1]);
    // Serial.print(" | ");
    // Serial.print(converter.data[2]);
    // Serial.print(" | ");
    // Serial.print(converter.data[3]);
    // Serial.print(" || ");

    converter.value1 = thermocouple.readInternal();
    CAN.sendMsgBuf(0x101, 0, 4, converter.data1);
    // Serial.print(converter.data1[0]);
    // Serial.print(" | ");
    // Serial.print(converter.data1[1]);
    // Serial.print(" | ");
    // Serial.print(converter.data1[2]);
    // Serial.print(" | ");
    // Serial.print(converter.data1[3]);
    // Serial.println(" | ");

    // digitalWrite(A6, HIGH);
    // delay(1000);
    // Serial.println("Busser ON");
    // digitalWrite(A6, LOW);
    // delay(5000);

    // rawData = myMAX31855.readRawData();

    // Serial.print(LaunchCount);
    // Serial.print(",");
    // Serial.println(WaitingCount);

    /*B3M テスト用*/

    // B3M_setPosition(0x01, -9000, 100); //B3Mを-90度(-9000)動作させる
    // delay(2100);
    // B3M_setPosition(0x01, 9000, 100);
    // delay(2100);

    /*B3M テスト用*/

    /*RS405CB テスト用*/

    // delay(100);
    // Move(1, 900, 50); // ID =1, GoalPosition = 90.0deg(900), Time = 0.5sec(50)
    // delay(2510);
    // Move(1, -900, 100); // ID =1, GoalPosition = -90.0deg(-900), Time = 1.0sec(100)
    // delay(2100);

    /*RS405CB テスト用*/
}

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

    for (int i = 0; i < 7; i++)
    {
        txCmd[7] += txCmd[i];
        Serial.print(txCmd[i]);
        Serial.print("");
    }
    Serial.println("");
    txCmd[7] = (byte)(txCmd[7]); // CHECKSUM

    // flag = B3M.synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
    flag = B3M.synchronize(txCmd, 8, rxCmd, 5);

    if (flag == false)
    {
        return -1;
        Serial.println(F("synchronize ERROR"));
    }
    reData = rxCmd[2];

    return reData;
}

int B3M_setPosition(byte id, int Pos, int Time)
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
        Serial.println(F("synchronize ERROR"));
    }
    reData = rxCmd[7];
    for (int i = 0; i < 7; i++)
    {
        Serial.print(reData);
        Serial.print(" ");
    }
    Serial.println("");

    return reData;
}

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

    digitalWrite(REDE, HIGH);
    for (int i = 0; i <= 8; i++)
    {
        Serial1.write(TxData[i]);
    }

    Serial1.flush();

    digitalWrite(REDE, LOW);
}

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
    digitalWrite(REDE, HIGH);
    for (int i = 0; i <= 11; i++)
    {
        Serial1.write(TxData[i]);
    }
    Serial1.flush();

    digitalWrite(REDE, LOW);
}

void B3M_initialize()
{
    // B3M_writeCommand(0x01, 0x02, 0x28); // 動作モード：Free
    Serial.println(B3M_writeCommand(0x01, 0x02, 0x28));

    // B3M_writeCommand(0x01, 0x02, 0x28); // 位置制御モードに設定
    Serial.println(B3M_writeCommand(0x01, 0x02, 0x28));

    // B3M_writeCommand(0x01, 0x01, 0x29); // 起動生成タイプ：Even
    Serial.println(B3M_writeCommand(0x01, 0x01, 0x29));

    // B3M_writeCommand(0x01, 0x00, 0x5c); // ゲインプリセット：No.0
    Serial.println(B3M_writeCommand(0x01, 0x00, 0x5c));

    // B3M_writeCommand(0x01, 0x00, 0x28); // 動作モード：Normal
    Serial.println(B3M_writeCommand(0x01, 0x00, 0x28));
}

// void MAX31855_initialize()
// {
//     myMAX31855.begin();
//     while (myMAX31855.getChipID() != MAX31855_ID)
//     {
//         Serial.println(F("MAX31855 error"));
//         delay(5000);
//     }
// }

// void MAX31855_errornotification()
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
//     }
// }

float CorrectedTemperature()
{
    const float VOLTAGE_PER_DEGREE = 41.276;
    float correcttemperature;

    float temperature = thermocouple.readCelsius();
    float coldtemperature = thermocouple.readInternal();
    float Voltage = VOLTAGE_PER_DEGREE*(temperature - coldtemperature);

    return correcttemperature = Voltage/VOLTAGE_PER_DEGREE + coldtemperature;
}

void SIGNAL_initialize()
{
    uint8_t LaunchPin = 7;
    uint8_t WaitingPin = 8;

    pinMode(LaunchPin, INPUT_PULLUP);
    pinMode(WaitingPin, INPUT_PULLUP);
}

// void Launch_Count()
// {
//     uint8_t Position;
//     uint8_t LaunchCount;
//     uint8_t LaunchPin;

//     if (Position == 1 && digitalRead(LaunchPin) == LOW)
//     {
//         LaunchCount++;
//     }
//     else
//     {
//         LaunchCount = 0;
//     }
// }