#include <TaskManager.h>
#include "IcsHardSerialClass.h"
#include <SPI.h>
#include <mcp2515_can.h>

/* CAN Config START */
const uint8_t SPI_CS_PIN = 6;
mcp2515_can CAN(SPI_CS_PIN);
/* CAN Config END */

/* CANDATA Config START */
namespace StateTransition
{
    enum class Mode : uint8_t
    {
        LAUNCH,
        WAITING,
    };

    Mode ChangeMode;
}

// union Converter
// {

// }converter;

/* CANDATA Config END */

/* B3M Servo Config START */
const byte ENABLE_PIN = 2;
const long BAUNDRATE = 115200;
const int TIMEOUT = 100;
IcsHardSerialClass B3M(&Serial1, ENABLE_PIN, BAUNDRATE, TIMEOUT);
/* B3M Servo Config END */

/* RS405CB Servo Config START */
uint8_t REDE_PIN = 3;
int rs405cbCloseangle = -800;
int rs405cbOpenangle = 0;
/* RS405CB Servo Config END */

/* State Transition Config START */
constexpr uint8_t POSITION_CHANGING_THRESHOLD = 5;
uint8_t WaitingCount = 0;
uint8_t LaunchCount = 0;
uint8_t Position = 1;
uint8_t WaitingPin = 7;
uint8_t LaunchPin = 8;
/* State Transition Config END */

/* Buzzer Config START */
const uint8_t BuzzerPin = A6;
/* Buzzer Config END */

/* LED Config START */
const uint8_t LaunchLED = A2;
const uint8_t WaitingLED = A3;
const uint8_t TasksLED = A7;
/* LED Config END */

void setup()
{
    Serial1.begin(115200, SERIAL_8N1);
    Serial1.begin(115200);
    Serial.begin(115200);

    pinMode(REDE_PIN, OUTPUT);
    Torque(0x01, 0x01);
    Move(1, rs405cbOpenangle, 100);

    SignalInitialize();
    StateTransition::ChangeMode = StateTransition::Mode::WAITING;

    B3M.begin();
    B3M_initialize();
    B3M_setposition(0x01, 0, 1000);

    CAN.begin(CAN_250KBPS, MCP_8MHz);

    BuzzerInitialize();
    LEDInitialize();

    Tasks.add("task", []()
              {
                  ToggleTasksLED();
                  ChangeLaunchMode();
                  ChangeWaitingMode();
                  Serial.print(F("launchCount: "));
                  Serial.print(LaunchCount);
                  Serial.print(F(" | "));
                  Serial.print(F("waitingCount: "));
                  Serial.print(WaitingCount);
                  Serial.print(F(" | "));
                  Serial.print(F("Mode: "));
                  if(StateTransition::ChangeMode == StateTransition::Mode::LAUNCH)
                  {
                    Serial.println(F("LAUNCH"));
                  }
                  else
                  {
                    Serial.println(F("WAITING"));
                  }; })
        ->startFps(100);

    Tasks.add("Buzzer", []()
              { ToggleBuzzer(); });

    Tasks["Buzzer"]->startIntervalMsecForCount(150, 6);
}

void loop()
{
    Tasks.update();

    CAN.sendMsgBuf(0x103, 0, 1, static_cast<uint8_t>(StateTransition::ChangeMode));
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

    digitalWrite(REDE_PIN, HIGH);
    for (int i = 0; i <= 8; i++)
    {
        Serial1.write(TxData[i]);
    }

    Serial1.flush();
    digitalWrite(REDE_PIN, LOW);
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
    digitalWrite(REDE_PIN, HIGH);
    for (int i = 0; i <= 11; i++)
    {
        Serial1.write(TxData[i]);
    }
    Serial1.flush();

    digitalWrite(REDE_PIN, LOW);
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

void ChangeLaunchMode()
{
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
        StateTransition::ChangeMode = StateTransition::Mode::LAUNCH;

        Move(1, -800, 10);
        delay(200);
        B3M_setposition(0x01, -6500, 10);

        Tasks["Buzzer"]->startIntervalMsecForCount(50, 6);

        LaunchCount = 0;
        Position = 2;

        digitalWrite(LaunchLED, HIGH);
        digitalWrite(WaitingLED, LOW);
    }
}

void ChangeWaitingMode()
{
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
        StateTransition::ChangeMode = StateTransition::Mode::WAITING;

        Move(1, 0, 100);
        delay(500);
        B3M_setposition(0x01, 0, 1000);

        Tasks["Buzzer"]->startIntervalMsecForCount(100, 4);

        WaitingCount = 0;
        Position = 1;

        digitalWrite(WaitingLED, HIGH);
        digitalWrite(LaunchLED, LOW);
    }
}

void SignalInitialize()
{
    pinMode(LaunchPin, INPUT_PULLUP);
    pinMode(WaitingPin, INPUT_PULLUP);
}

void BuzzerInitialize()
{
    pinMode(BuzzerPin, OUTPUT);
}

void LEDInitialize()
{
    pinMode(WaitingLED, OUTPUT);
    pinMode(LaunchLED, OUTPUT);
    pinMode(TasksLED, OUTPUT);

    digitalWrite(LaunchLED, LOW);
    digitalWrite(WaitingLED, HIGH);
}

void ToggleTasksLED()
{
    digitalWrite(TasksLED, !digitalRead(TasksLED));
}

void ToggleBuzzer()
{
    digitalWrite(BuzzerPin, !digitalRead(BuzzerPin));
}