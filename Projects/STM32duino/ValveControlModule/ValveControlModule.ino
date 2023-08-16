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

namespace Error
{
    enum class ErrorCode : uint8_t
    {
        MOTOR_POSITION_FAILED,
    };

    ErrorCode ErrorCode;
}

union Converter
{
    uint8_t data[6];
    int16_t data_16[3];
} converter;

/* CANDATA Config END */

/* B3M Servo Config START */
const byte ENABLE_PIN = 2;
const long BAUNDRATE = 115200;
const int TIMEOUT = 500;
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
    Serial1.begin(115200);
    Serial1.begin(115200, SERIAL_8N1);

    Serial.begin(115200);

    pinMode(REDE_PIN, OUTPUT);
    Torque(0x01, 0x01);
    Move(1, rs405cbOpenangle, 100);

    SignalInitialize();
    StateTransition::ChangeMode = StateTransition::Mode::WAITING;

    B3M.begin();
    B3M_initialize();
    B3M_setposition(0x01, 0, 1000);

    CAN.begin(CAN_1000KBPS, MCP_8MHz); // CAN_1000Kbps, MCP_8MHzに設定

    // デバック用
    //  while (CAN_OK != CAN.begin(CAN_250KBPS, MCP_8MHz))
    //  {
    //      Serial.println("CAN init fail, retry...");
    //      delay(100);
    //  }
    //  Serial.println("CAN init ok!");

    BuzzerInitialize();
    LEDInitialize();

    Tasks.add("CAN", []()
              {
                  ToggleTasksLED();
                  Serial.print(" |Error-Sta|> ");
                  Serial.print(b3mReaderrorStatus(0x01));
                  Serial.print(" |System-Error-Sta|> ");
                  Serial.print(b3mReadsystemError(0x01));
                  Serial.print(" |Motor-Sta|> ");
                  Serial.print(b3mReadmotorStatus(0x01));
                  Serial.print(" |Uart-Rece-Error|> ");
                  Serial.print(b3mReaduartReceptionerror(0x01));
                  Serial.print(" |Command-Error|> ");
                  Serial.println(b3mReadcommandError(0x01));

                  uint32_t currentTime = millis();

                  //   147~149の処理で特に問題が無ければ削除
                    int16_t recieveCurrentposition = b3mReadcurrentPosition(0x01);
                    int16_t recieveDesiredPosition = b3mReaddesiredPosition(0x01);
                    int16_t recieveCurrentvelosity = b3mReadcurrentVelosity(0x01);
                    uint8_t recieveCurrent = b3mReadcurrent(0x01);
                    uint8_t recieveMcutemperature = b3mReadmcuTemperature(0x01);
                    uint8_t recieveMotortemperature = b3mReadmotorTemperature(0x01);
                    uint8_t recieveinputVoltage = b3mreadInputvoltage(0x01);
                    // uint8_t recieveStatus = b3mreadStatus(0x01);

                  //   Serial.print(recieveCurrentposition);
                  //   Serial.print(" <| ");
                  //   Serial.print(recieveDesiredPosition);
                  //   Serial.print(" <| ");
                  //   Serial.print(recieveCurrentvelosity);
                  //   Serial.print(" <| ");
                  //   Serial.print(recieveStatus);
                  //   Serial.print(" <| ");
                  //   Serial.print(recieveMotortemperature);
                  //   Serial.print(" <| ");
                  //   Serial.print(recieveMcutemperature);
                  //   Serial.print(" <| ");
                  // Serial.print(b3mReadcurrent(0x01));
                  // Serial.print(" <| ");
                  //   Serial.print(recieveinputVoltage);
                  //   Serial.print(" <| ");

                    canSendmcuInfomation(recieveMotortemperature, recieveMcutemperature, recieveCurrent, recieveinputVoltage);
                    canSendmotorInfomation(recieveCurrentposition, recieveDesiredPosition, recieveCurrentvelosity);
                //   canSendmcuInfomation(b3mReadmotorTemperature(0x01), b3mReadmcuTemperature(0x01), b3mReadcurrent(0x01), b3mreadInputvoltage(0x01));
                //   canSendmotorInfomation(b3mReadcurrentPosition(0x01), b3mReaddesiredPosition(0x01), b3mReadcurrentVelosity(0x01)); 
                //   canSendservoStatusinfomation(b3mReaderrorStatus(0x01), b3mReadsystemError(0x01), b3mReadmotorStatus(0x01), b3mReaduartReceptionerror(0x01), b3mReadcommandError(0x01));
                  canSendmotorStatus(b3mReadmotorStatus(0x01), currentTime);
                  })
        ->startFps(13);

    // Tasks.add("Count", []()
    //           {
    //             Serial.print(F("Mode: "));
    //             if (StateTransition::ChangeMode == StateTransition::Mode::LAUNCH)
    //             {
    //                 Serial.print(F("LAUNCH"));
    //                 Serial.print(F(" | "));
    //             }
    //             else
    //             {
    //                 Serial.print(F("WAITING"));
    //                 Serial.print(F(" | "));

    //             };
    //             Serial.print(F("launchCount: "));
    //             Serial.print(LaunchCount);
    //             Serial.print(F(" | "));
    //             Serial.print(F("waitingCount: "));
    //             Serial.print(WaitingCount);
    //             Serial.print(F(" | "));
    //             Serial.println(""); })
    // ->startFps(100);

    Tasks.add("Mode", []()
              {
                  uint8_t can[1];
                  can[0] = static_cast<uint8_t>(StateTransition::ChangeMode);
                  CAN.sendMsgBuf(0x10A, 0, 1, can);
                  // Serial.print(can[0]);
                  // Serial.println(" | ");
              })
        ->startFps(59);

    // Tasks.add("Buzzer", []()
    //           { ToggleBuzzer(); });

    /* Wake Up Buzzer */
    // Tasks["Buzzer"]->startIntervalMsecForCount(66, 10);
}

void loop()
{
    Tasks.update();
    ChangeLaunchMode();
    ChangeWaitingMode();
}

void canSendmode()
{
    uint8_t can[1];
    can[0] = static_cast<uint8_t>(StateTransition::ChangeMode);
    CAN.sendMsgBuf(0x10A, 0, 1, can);
    Serial.print(can[0]);
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

void B3M_initialize()
{
    B3M_writeCommand(0x01, 0x02, 0x28); // 動作モード：Free
    // Serial.println(B3M_writeCommand(0x01, 0x02, 0x28));

    B3M_writeCommand(0x01, 0x02, 0x28); // 位置制御モードに設定
    // Serial.println(B3M_writeCommand(0x01, 0x02, 0x28));

    B3M_writeCommand(0x01, 0x01, 0x29); // 起動生成タイプ：Even
    // Serial.println(B3M_writeCommand(0x01, 0x01, 0x29));

    B3M_writeCommand(0x01, 0x00, 0x5c); // ゲインプリセット：No.0
    // Serial.println(B3M_writeCommand(0x01, 0x00, 0x5c));

    B3M_writeCommand(0x01, 0x00, 0x28); // 動作モード：Normal
    // Serial.println(B3M_writeCommand(0x01, 0x00, 0x28));
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

        Move(1, -800, 10); //-800/10 = -80deg
        delay(100);
        B3M_setposition(0x01, -5500, 10); //-6500/100 = -65deg
        // Tasks["Buzzer"]->startIntervalMsecForCount(50, 6);

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
        delay(100);
        B3M_setposition(0x01, 0, 1000);
        // Tasks["Buzzer"]->startIntervalMsecForCount(100, 4);

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

int16_t b3mReaddesiredPosition(byte id)
{
    byte txCmd[7];
    byte rxCmd[7];
    int16_t value;
    int16_t reData;
    bool flag;

    txCmd[0] = (byte)(0x07); // SIZE
    txCmd[1] = (byte)(0x03); // COMMAND
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(0x2A); // ADDRESS //目標位置のアドレス
    txCmd[5] = (byte)(0x02); // LENGTH

    txCmd[6] = 0x00; // SUM

    for (int i = 0; i < 6; i++)
    {
        txCmd[6] += txCmd[i];
        // Serial.print(txCmd[i]);
        // Serial.print("|");
    }
    // Serial.println("");

    txCmd[6] = (byte)(txCmd[6]);
    flag = B3M.synchronize(txCmd, 7, rxCmd, 7);

    if (flag == false)
    {
        return -1;
    }

    // Serial.print("RAM: ");
    for (int o = 4; o < 6; o++)
    {
        value = (rxCmd[o] << 8) | rxCmd[o];
        // Serial.print(rxCmd[o], HEX);
        // Serial.print(" ");
    }
    // Serial.print(" | ");

    reData = (value & 0xFF) << 8 | (value >> 8 & 0xFF);

    return reData / 100;
}

int16_t b3mReadcurrentPosition(byte id)
{
    byte txCmd[7];
    byte rxCmd[7];
    int16_t value;
    int16_t reData;
    bool flag;

    txCmd[0] = (byte)(0x07); // SIZE
    txCmd[1] = (byte)(0x03); // COMMAND
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(0x2C); // ADDRESS  //現在位置のアドレス
    txCmd[5] = (byte)(0x02); // LENGTH

    txCmd[6] = 0x00; // SUM

    for (int i = 0; i < 6; i++)
    {
        txCmd[6] += txCmd[i];
        // Serial.print(txCmd[i]);
        // Serial.print("|");
    }
    // Serial.println("");

    txCmd[6] = (byte)(txCmd[6]);
    flag = B3M.synchronize(txCmd, 7, rxCmd, 7);

    if (flag == false)
    {
        return -1;
    }

    // Serial.print("RAM: ");
    for (int o = 4; o < 6; o++)
    {
        value = (rxCmd[o] << 8) | rxCmd[o];
        // Serial.print(rxCmd[o], HEX);
        // Serial.print(" ");
    }
    // Serial.print(" | ");

    reData = (value & 0xFF) << 8 | (value >> 8 & 0xFF);

    return reData / 100;
}

int16_t b3mReadcurrentVelosity(byte id)
{
    byte txCmd[7];
    byte rxCmd[7];
    int16_t value;
    int16_t reData;
    bool flag;

    txCmd[0] = (byte)(0x07); // SIZE
    txCmd[1] = (byte)(0x03); // COMMAND
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(0x32); // ADDRESS  //現在速度のアドレス
    txCmd[5] = (byte)(0x02); // LENGTH

    txCmd[6] = 0x00; // SUM

    for (int i = 0; i < 6; i++)
    {
        txCmd[6] += txCmd[i];
        // Serial.print(txCmd[i]);
        // Serial.print("|");
    }
    // Serial.println("");

    txCmd[6] = (byte)(txCmd[6]);
    flag = B3M.synchronize(txCmd, 7, rxCmd, 7);

    if (flag == false)
    {
        return -1;
    }

    // Serial.print("RAM: ");
    for (int o = 4; o < 6; o++)
    {
        value = (rxCmd[o] << 8) | rxCmd[o];
        // Serial.print(rxCmd[o], HEX);
        // Serial.print(" ");
    }
    // Serial.print(" | ");

    reData = (value & 0xFF) << 8 | (value >> 8 & 0xFF);

    return reData / 100;
}

int16_t b3mReadmcuTemperature(byte id)
{
    byte txCmd[7];
    byte rxCmd[7];
    int16_t value;
    int16_t reData;
    bool flag;

    txCmd[0] = (byte)(0x07); // SIZE
    txCmd[1] = (byte)(0x03); // COMMAND
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(0x44); // ADDRESS  //現在のMCU温度のアドレス
    txCmd[5] = (byte)(0x02); // LENGTH

    txCmd[6] = 0x00; // SUM

    for (int i = 0; i < 6; i++)
    {
        txCmd[6] += txCmd[i];
        // Serial.print(txCmd[i]);
        // Serial.print("|");
    }
    // Serial.println("");

    txCmd[6] = (byte)(txCmd[6]);
    flag = B3M.synchronize(txCmd, 7, rxCmd, 7);

    if (flag == false)
    {
        return -1;
    }

    // Serial.print("RAM: ");
    for (int o = 4; o < 6; o++)
    {
        value = (rxCmd[o] << 8) | rxCmd[o];
        // Serial.print(rxCmd[o], HEX);
        // Serial.print(" ");
    }
    // Serial.print(" | ");

    reData = (value & 0xFF) << 8 | (value >> 8 & 0xFF);

    return reData / 100;
}

int16_t b3mReadmotorTemperature(byte id)
{
    byte txCmd[7];
    byte rxCmd[7];
    int16_t value;
    int16_t reData;
    bool flag;

    txCmd[0] = (byte)(0x07); // SIZE
    txCmd[1] = (byte)(0x03); // COMMAND
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(0x46); // ADDRESS  //現在のモーター温度のアドレス
    txCmd[5] = (byte)(0x02); // LENGTH

    txCmd[6] = 0x00; // SUM

    for (int i = 0; i < 6; i++)
    {
        txCmd[6] += txCmd[i];
        // Serial.print(txCmd[i]);
        // Serial.print("|");
    }
    // Serial.println("");

    txCmd[6] = (byte)(txCmd[6]);
    flag = B3M.synchronize(txCmd, 7, rxCmd, 7);

    if (flag == false)
    {
        return -1;
    }

    // Serial.print("RAM: ");
    for (int o = 4; o < 6; o++)
    {
        value = (rxCmd[o] << 8) | rxCmd[o];
        // Serial.print(rxCmd[o], HEX);
        // Serial.print(" ");
    }
    // Serial.print(" | ");

    reData = (value & 0xFF) << 8 | (value >> 8 & 0xFF);

    return reData / 100;
}

int16_t b3mReadcurrent(byte id)
{
    byte txCmd[7];
    byte rxCmd[7];
    int16_t value;
    int16_t reData;
    bool flag;

    txCmd[0] = (byte)(0x07); // SIZE
    txCmd[1] = (byte)(0x03); // COMMAND
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(0x48); // ADDRESS  //現在負荷電流値のアドレス
    txCmd[5] = (byte)(0x02); // LENGTH

    txCmd[6] = 0x00; // SUM

    for (int i = 0; i < 6; i++)
    {
        txCmd[6] += txCmd[i];
        // Serial.print(txCmd[i]);
        // Serial.print("|");
    }
    // Serial.println("");

    txCmd[6] = (byte)(txCmd[6]);
    flag = B3M.synchronize(txCmd, 7, rxCmd, 7);

    if (flag == false)
    {
        return -1;
    }

    // Serial.print("RAM: ");
    for (int o = 4; o < 6; o++)
    {
        value = (rxCmd[o] << 8) | rxCmd[o];
        // Serial.print(rxCmd[o], HEX);
        // Serial.print(" ");
    }
    // Serial.print(" | ");

    reData = (value & 0xFF) << 8 | (value >> 8 & 0xFF);

    return reData / 100;
}

int16_t b3mreadInputvoltage(byte id)
{
    byte txCmd[7];
    byte rxCmd[7];
    int16_t value;
    int16_t reData;
    bool flag;

    txCmd[0] = (byte)(0x07); // SIZE
    txCmd[1] = (byte)(0x03); // COMMAND
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(0x4A); // ADDRESS  //現在負荷電流値のアドレス
    txCmd[5] = (byte)(0x02); // LENGTH

    txCmd[6] = 0x00; // SUM

    for (int i = 0; i < 6; i++)
    {
        txCmd[6] += txCmd[i];
        // Serial.print(txCmd[i]);
        // Serial.print("|");
    }
    // Serial.println("");

    txCmd[6] = (byte)(txCmd[6]);
    flag = B3M.synchronize(txCmd, 7, rxCmd, 7);

    if (flag == false)
    {
        return -1;
    }

    // Serial.print("RAM: ");
    for (int o = 4; o < 6; o++)
    {
        value = (rxCmd[o] << 8) | rxCmd[o];
        // Serial.print(rxCmd[o], HEX);
        // Serial.print(" ");
    }
    // Serial.print(" | ");

    reData = (value & 0xFF) << 8 | (value >> 8 & 0xFF);

    return reData / 100;
}

uint8_t b3mReaderrorStatus(byte id)
{
    byte txCmd[7];
    byte rxCmd[6];
    // uint8_t value;
    uint8_t reData;
    bool flag;

    txCmd[0] = (byte)(0x07); // SIZE
    txCmd[1] = (byte)(0x03); // COMMAND
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(0x9D); // ADDRESS  //エラーステータスを取得
    txCmd[5] = (byte)(0x01); // LENGTH

    txCmd[6] = 0x00; // SUM

    for (int i = 0; i < 6; i++)
    {
        txCmd[6] += txCmd[i];
    }

    txCmd[6] = (byte)(txCmd[6]);
    flag = B3M.synchronize(txCmd, 7, rxCmd, 6);

    if (flag == false)
    {
        return -1;
    }

    reData = rxCmd[4];
    return reData;
}

uint8_t b3mReadsystemError(byte id)
{
    byte txCmd[7];
    byte rxCmd[6];
    // uint8_t value;
    uint8_t reData;
    bool flag;

    txCmd[0] = (byte)(0x07); // SIZE
    txCmd[1] = (byte)(0x03); // COMMAND
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(0x9E); // ADDRESS  //システムエラーを取得
    txCmd[5] = (byte)(0x01); // LENGTH

    txCmd[6] = 0x00; // SUM

    for (int i = 0; i < 6; i++)
    {
        txCmd[6] += txCmd[i];
    }

    txCmd[6] = (byte)(txCmd[6]);
    flag = B3M.synchronize(txCmd, 7, rxCmd, 6);

    if (flag == false)
    {
        return -1;
    }

    reData = rxCmd[4];
    return reData;
}

uint8_t b3mReadmotorStatus(byte id)
{
    byte txCmd[7];
    byte rxCmd[6];
    // uint8_t value;
    uint8_t reData;
    bool flag;

    txCmd[0] = (byte)(0x07); // SIZE
    txCmd[1] = (byte)(0x03); // COMMAND
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(0x9F); // ADDRESS  //モーターステータス異常を取得
    txCmd[5] = (byte)(0x01); // LENGTH

    txCmd[6] = 0x00; // SUM

    for (int i = 0; i < 6; i++)
    {
        txCmd[6] += txCmd[i];
    }

    txCmd[6] = (byte)(txCmd[6]);
    flag = B3M.synchronize(txCmd, 7, rxCmd, 6);

    if (flag == false)
    {
        return -1;
    }

    reData = rxCmd[4];
    return reData;
}

uint8_t b3mReaduartReceptionerror(byte id)
{
    byte txCmd[7];
    byte rxCmd[6];
    // uint8_t value;
    uint8_t reData;
    bool flag;

    txCmd[0] = (byte)(0x07); // SIZE
    txCmd[1] = (byte)(0x03); // COMMAND
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(0xA0); // ADDRESS  //UART受信エラーを取得
    txCmd[5] = (byte)(0x01); // LENGTH

    txCmd[6] = 0x00; // SUM

    for (int i = 0; i < 6; i++)
    {
        txCmd[6] += txCmd[i];
    }

    txCmd[6] = (byte)(txCmd[6]);
    flag = B3M.synchronize(txCmd, 7, rxCmd, 6);

    if (flag == false)
    {
        return -1;
    }

    reData = rxCmd[4];
    return reData;
}

uint8_t b3mReadcommandError(byte id)
{
    byte txCmd[7];
    byte rxCmd[6];
    // uint8_t value;
    uint8_t reData;
    bool flag;

    txCmd[0] = (byte)(0x07); // SIZE
    txCmd[1] = (byte)(0x03); // COMMAND
    txCmd[2] = (byte)(0x00); // OPTION
    txCmd[3] = (byte)(id);   // ID

    txCmd[4] = (byte)(0xA1); // ADDRESS  //コマンドエラーを取得
    txCmd[5] = (byte)(0x01); // LENGTH

    txCmd[6] = 0x00; // SUM

    for (int i = 0; i < 6; i++)
    {
        txCmd[6] += txCmd[i];
    }

    txCmd[6] = (byte)(txCmd[6]);
    flag = B3M.synchronize(txCmd, 7, rxCmd, 6);

    if (flag == false)
    {
        return -1;
    }

    reData = rxCmd[4];
    return reData;
}

int16_t uint8_to_int16(uint8_t *array)
{
    // uint8_t型の配列をint16_t型の値に変換する
    int16_t value;
    memcpy(&value, array, sizeof(value));
    return value;
}

void canSendmcuInfomation(uint8_t motorTemperature, uint8_t mcuTemperature, uint8_t current, uint8_t inputVoltage)
{
    uint8_t data[4];
    memcpy(data, &motorTemperature, sizeof(motorTemperature));
    memcpy(data + sizeof(mcuTemperature), &mcuTemperature, sizeof(mcuTemperature));
    memcpy(data + sizeof(mcuTemperature) + sizeof(current), &current, sizeof(current));
    memcpy(data + sizeof(mcuTemperature) + sizeof(current) + sizeof(inputVoltage), &inputVoltage, sizeof(inputVoltage));

    CAN.sendMsgBuf(0x10B, 0, 4, data);

    // デバック用
    //  memcpy(&motorTemperature, data, sizeof(motorTemperature));
    //  memcpy(&mcuTemperature, data + sizeof(mcuTemperature), sizeof(mcuTemperature));
    //  Serial.print(data[0]);
    //  Serial.print(" | ");
    //  Serial.print(data[1]);
    //  Serial.print(" | ");
    //  Serial.print(data[2]);
    //  Serial.print(" | ");
    //  Serial.print(data[3]);
    //  Serial.println(" | ");
}

void canSendmotorInfomation(int16_t currentposition, int16_t currentDesiredposition, int16_t currentVelosity)
{
    converter.data_16[0] = currentposition;
    converter.data_16[1] = currentDesiredposition;
    converter.data_16[2] = currentVelosity;

    CAN.sendMsgBuf(0x10C, 0, 6, converter.data);

    int16_t currentposition_back = uint8_to_int16(converter.data);
    int16_t currentDesiredposition_back = uint8_to_int16(converter.data + 2);
    int16_t currentVelosity_back = uint8_to_int16(converter.data + 4);

    // デバック用
    // Serial.print(currentposition_back);
    // Serial.print(" | ");
    // Serial.print(currentDesiredposition_back);
    // Serial.print(" | ");
    // Serial.print(currentVelosity_back);
    // Serial.println(" | ");
}

void canSendservoStatusinfomation(uint8_t errorStatus, uint8_t systemError, uint8_t motorStatus, uint8_t uartReceptionerror, uint8_t commandError)
{
    uint8_t data[5];
    memcpy(data, &errorStatus, sizeof(errorStatus));
    memcpy(data + sizeof(errorStatus), &systemError, sizeof(systemError));
    memcpy(data + sizeof(errorStatus) + sizeof(systemError), &motorStatus, sizeof(motorStatus));
    memcpy(data + sizeof(errorStatus) + sizeof(systemError) + sizeof(motorStatus), &uartReceptionerror, sizeof(uartReceptionerror));
    memcpy(data + sizeof(errorStatus) + sizeof(systemError) + sizeof(motorStatus) + sizeof(uartReceptionerror), &commandError, sizeof(commandError));

    CAN.sendMsgBuf(0x107, 0, 5, data);
}

//1byte目 5
//2byte目 errorcode :
//3byte目 errorreason :FF
//4byte目 millis()
//5byte目 millis()

void canSendservoStatus(uint8_t servoStatus)
{
    uint8_t data[1];
    memcpy(data, &servoStatus, 1);
    CAN.sendMsgBuf(0x106, 0, 1, data);
}

void canSendmotorStatus(uint8_t motorStatus, uint32_t currentTime)
{
    if(motorStatus == 0)return;
    uint8_t data[7];
    data[0] = 5;
    memcpy(data + 1, &motorStatus, sizeof(motorStatus));
    data[2] = 0;
    memcpy(data + 3, &currentTime, sizeof(currentTime));

    CAN.sendMsgBuf(0x07, 0, 7, data);
}

void canSendcommandError(uint8_t uartReceptionerror, uint8_t currentTime)
{
    if(uartReceptionerror == 0)return;
    uint8_t data[7];
    data[0] = 5; //valvecontroler君
    memcpy(data + 1, &uartReceptionerror, sizeof(uartReceptionerror));
    data[2] = 0;
    memcpy(data + 3, &currentTime, sizeof(currentTime));

    CAN.sendMsgBuf(0x07, 0, 7, data);
}


// 関数の定義
// int16_t getServoPosition() {
//   return ポジションを取得する処理
// }

// void canSendPostion(int16_t position) {
//   uint8_t data[2];
//   memcpy(data, &position, 2);

//   _can->sendMsgBuf(0x200, 0, 2, data);
// }

// // loopの中
// int16_t receivedPosition = getServoPosition();
// canSendPostion(receivedPosition);
// Serial.println(receivedPosition);

// エラーデータ送信
// TODO