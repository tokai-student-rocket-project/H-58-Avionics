#include <TaskManager.h>
#include "IcsHardSerialClass.h"
// #include "EX_MAX31855.h" //test用のhファイル
#include "Adafruit_MAX31855.h"
#include <SPI.h>
#include <mcp2515_can.h>

/* VV namespaceをつかう*/

/* CAN Config START */
const int SPI_CS_PIN = 6;
mcp2515_can CAN(SPI_CS_PIN);
// unsigned char sample[8] = {0, 0, 0, 0, 0, 0, 0, 0};

namespace event
{
    enum class Mode : uint8_t
    {
        LAUNCH,
        WAITING,
    };

    Mode eventMode;
}
union Converter
{
    double correctedTemperature, coldJunctiontemperature, thermoCoupletemperature;
    uint8_t correctedTemperatureData[8], coldJunctiontemperatureData[8], thermoCoupletemperatureData[8];
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
int rs405cbCloseangle = -800;
int rs405cbOpenangle = 0;

/*RS405CB Servo Config END*/

/*Mode Change Config START*/

constexpr int POSITION_CHANGING_THRESHOLD = 5;
uint8_t launchCount = 0;
uint8_t waitingCount = 0;
uint8_t position = 1;
uint8_t waitingPin = 8;
uint8_t launchPin = 7;

/* Change Config END*/

/*MAX31855 Config START*/

Adafruit_MAX31855 thermocouple(5);

/*MAX31855 Config END*/

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
    Serial1.begin(115200, SERIAL_8N1); // 通信速度、パリティなしに設定
    Serial1.begin(115200);
    Serial.begin(115200);
    Serial.println(F("Serial OK"));

    /* --- RS405CB Config --- */

    pinMode(REDE, OUTPUT);
    Torque(0x01, 0x01);
    delay(100);
    Move(1, rs405cbOpenangle, 100);

    /* --- RS405CB END Config --- */

    SignalInitialize();
    event::eventMode = event::Mode::WAITING;

    /* ---B3M Config--- */

    B3M.begin(); // B3Mと通信開始
    B3M_initialize();
    B3M_setposition(0x01, 0, 1000);

    /* ---B3M Config END--- */

    /* --- CAN --- */

    CAN.begin(CAN_500KBPS, MCP_8MHz); // 500KBPS　<- 500kBpsに設定, MCP_8MHz <- MCP2515に搭載する水晶発振子に依存 //この値に固定

    /* --- CAN Config END --- */

    /* --- MAX31855 ---*/

    thermocouple.begin();

    /* ---  MAX31855 Config END ---*/

    /* --- BUZZER Config START --- */

    pinMode(A6, OUTPUT);

    /* --- BUZZER Config END --- */

    pinMode(A2, OUTPUT); // waitingLED
    pinMode(A3, OUTPUT); // launchLED
    pinMode(A7, OUTPUT); // tasksLED

    digitalWrite(A2, LOW); //launch LED OFF
    digitalWrite(A3, HIGH); //waiting LED ON

    Tasks.add("task", []()
              {
                ToggleTasksLED();
                Serial.print(F("CorrectedTemperature: "));
                Serial.print(CorrectedTemperature());
                Serial.print(F(" | "));
                Serial.print(F("ColdJunctionTemperature: "));
                Serial.print(thermocouple.readInternal());
                Serial.print(F(" | "));
                Serial.print(F("Temperature: "));
                Serial.print(thermocouple.readCelsius());
                Serial.print(F(" | "));
                ChangeLaunchMode();
                ChangeWaitingMode();
                Serial.print(F("launchCount: "));
                Serial.print(launchCount);
                Serial.print(F(" | "));
                Serial.print(F("waitingCount: "));
                Serial.print(waitingCount);
                Serial.print(F(" | "));
                Serial.print(F("Mode: "));
                if(event::eventMode == event::Mode::WAITING)
                {
                    Serial.println("WAITING");
                }
                else
                {
                    Serial.println("LAUNCH");
                }; })
        ->startFps(100);

    // Tasks.add("Buzzer", []()
    //           {
    //             if(event::eventMode == event::Mode::WAITING)
    //             {
    //                 //Serial.println("WAITING");
    //             }
    //             else
    //             {
    //                 //Serial.println("LAUNCH");
    //                 ToggleLaunchBuzzer();
    //             }; })
    //     //->startFpsForFrame(10, 5, false);
    //     ->startIntervalSecFromForCount(5, 1, 5, true);

    // Tasks.add("TasksLED", []()
    // {
    //     ToggleTasksLED();
    // }
    // )
    // ->startFps(30);
}

void loop()
{
    Tasks.update();

    // send data:  id = 0x100, standrad frame, data len = 8, stmp: data buf
    converter.correctedTemperature = CorrectedTemperature();
    CAN.sendMsgBuf(0x100, 0, 8, converter.correctedTemperatureData);
    // Serial.print(converter.correctedTemperatureData[0]);
    // Serial.print(converter.correctedTemperatureData[1]);
    // Serial.print(converter.correctedTemperatureData[2]);
    // Serial.print(converter.correctedTemperatureData[3]);
    // Serial.print(converter.correctedTemperatureData[4]);
    // Serial.print(converter.correctedTemperatureData[5]);
    // Serial.print(converter.correctedTemperatureData[6]);
    // Serial.println(converter.correctedTemperatureData[7]);

    converter.coldJunctiontemperature = thermocouple.readInternal();
    CAN.sendMsgBuf(0x101, 0, 8, converter.coldJunctiontemperatureData);

    converter.thermoCoupletemperature = thermocouple.readCelsius();
    CAN.sendMsgBuf(0x102, 0, 8, converter.thermoCoupletemperatureData);

    CAN.sendMsgBuf(0x103, 0, 1, static_cast<uint8_t>(event::eventMode));

    /*B3M テスト用*/

    // B3M_setposition(0x01, -9000, 100); //B3Mを-90度(-9000)動作させる
    // delay(2100);
    // B3M_setposition(0x01, 9000, 100);
    // delay(2100);

    /*B3M テスト用*/

    /*RS405CB テスト用*/

    // delay(100);
    // Move(1, 900, 50); // ID =1, Goalposition = 90.0deg(900), Time = 0.5sec(50)
    // delay(2510);
    // Move(1, -900, 100); // ID =1, Goalposition = -90.0deg(-900), Time = 1.0sec(100)
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

float CorrectedTemperature()
{
    const float VOLTAGE_PER_DEGREE = 41.276;
    float correcttemperature;

    float temperature = thermocouple.readCelsius();
    float coldtemperature = thermocouple.readInternal();
    float Voltage = VOLTAGE_PER_DEGREE * (temperature - coldtemperature);

    return correcttemperature = Voltage / VOLTAGE_PER_DEGREE + coldtemperature;
}

void SignalInitialize()
{
    uint8_t launchPin = 7;
    uint8_t waitingPin = 8;

    pinMode(launchPin, INPUT_PULLUP);
    pinMode(waitingPin, INPUT_PULLUP);
}

void ChangeLaunchMode()
{
    if (position == 1 && digitalRead(launchPin) == LOW)
    {
        launchCount++;
    }
    else
    {
        launchCount = 0;
    }

    if (launchCount >= POSITION_CHANGING_THRESHOLD)
    {
        launchCount = 0;
        event::eventMode = event::Mode::LAUNCH;
        Torque(0x01, 0x01);
        Move(1, -800, 10);
        delay(200);
        B3M_setposition(0x01, -6500, 10);

        position = 2;

        // digitalWrite(A6, HIGH);
        // delay(40);
        // digitalWrite(A6, LOW);
        // delay(40);
        // digitalWrite(A6, HIGH);
        // delay(40);
        // digitalWrite(A6, LOW);
        // delay(40);
        // digitalWrite(A6, HIGH);
        // delay(40);
        // digitalWrite(A6, LOW);
        digitalWrite(A3, LOW);
        digitalWrite(A2, HIGH);
    }
}

void ChangeWaitingMode()
{
    if (position == 2 && digitalRead(waitingPin) == LOW)
    {
        waitingCount++;
    }
    else
    {
        waitingCount = 0;
    }

    if (waitingCount >= POSITION_CHANGING_THRESHOLD)
    {
        waitingCount = 0;
        event::eventMode = event::Mode::WAITING;

        Move(1, 0, 100);
        delay(500);
        B3M_setposition(0x01, 0, 1000);

        position = 1;
        // digitalWrite(A6, HIGH);
        // delay(40);
        // digitalWrite(A6, LOW);
        // delay(40);
        // digitalWrite(A6, HIGH);
        // delay(40);
        // digitalWrite(A6, LOW);
        digitalWrite(A2, LOW);
        digitalWrite(A3, HIGH);
    }
}

bool BuzzerOn = false;
void ToggleLaunchBuzzer()
{
    BuzzerOn = !BuzzerOn;

    if (BuzzerOn)
    {
        digitalWrite(A6, HIGH);
    }
    else
    {
        digitalWrite(A6, LOW);
    }
}

bool TasksLEDOn = false;
void ToggleTasksLED()
{
    TasksLEDOn = !TasksLEDOn;

    if (TasksLEDOn)
    {
        digitalWrite(A7, HIGH);
    }
    else{
        digitalWrite(A7, LOW);
    }
}