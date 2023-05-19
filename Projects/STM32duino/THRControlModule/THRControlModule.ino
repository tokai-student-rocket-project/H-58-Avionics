#include <TaskManager.h>
#include "IcsHardSerialClass.h"
#include "EX_MAX31855.h" //test用のhファイル
#include <SPI.h>
#include <mcp2515_can.h>

/* CAN Config START */
const int SPI_CS_PIN = 6;
mcp2515_can CAN(SPI_CS_PIN);
unsigned char sample[8] = {0, 0, 0, 0, 0, 0, 0, 0};
union Converter
{
    float value;
    uint8_t data[4];
} converter;
/* CAN Config END */

/*B3M Servo Config START*/
const byte EN_PIN = 2;
const long BAUNDRATE = 115200;
const int TIMEOUT = 500;
// HardwareSerial SerialX(RX, TX); X = 1, 2, 3...
// HardwareSerial Serial1(PA_10, PA_9); // STM32F303K8
// HardwareSerial Serial1(0, 1); // STM32F303K8
IcsHardSerialClass B3M(&Serial1, EN_PIN, BAUNDRATE, TIMEOUT);
/*B3M Servo Config END*/

/*RS405CB Servo Config START*/
int REDE = 7;
/*RS405CB Servo Config END*/

/*Position Change Config START*/
constexpr int POSITION_CHANGING_THRESHOLD = 10;
int Launch_Count = 0;
int Waiting_Count = 0;
int Position = 1;
int WaitingPin = 4;
int LaunchPin = 5;
/*Position Change Config END*/

/*MAX31855 Config START*/
int32_t rawData = 0;
float temperature;
MAX31855 myMAX31855(3); //Chip Select PIN (CS)
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
    pinMode(A0, INPUT_PULLUP);


    pinMode(REDE, OUTPUT);
    Toque(0x01, 0x01);
    delay(100);
    Move(1, 0, 50);

    SIGNAL_initialize();
    MAX31855_initialize();

    B3M.begin(); // B3Mと通信開始
    Serial1.begin(115200, SERIAL_8N1); // 通信速度、パリティなしに設定
    Serial1.begin(115200);
    Serial.begin(115200);

    B3M_initialize();
    B3M_setPosition(0x01, 4500, 1000);

    /* --- CAN --- */

    while (!Serial)
    {
    };

    while (CAN_OK != CAN.begin(CAN_500KBPS))
    {
        Serial.println("CAN init fail, retry...");
        delay(100);
    }
    Serial.println("CAN init OK!");

    /* --- CAN --- */

    Tasks.add("task", []()
              {
                    Serial.print(myMAX31855.getTemperature(rawData));
                    Serial.print(", ");
                    Serial.print(myMAX31855.getColdJunctionTemperature(rawData));
                    Serial.print(", ");
                    Serial.print(Launch_Count);
                    Serial.print(", ");
                    Serial.println(Waiting_Count); })
        ->startIntervalMsec(5);
}

void loop()
{
    Tasks.update();

    rawData = myMAX31855.readRawData();

    if (Position == 1 && digitalRead(LaunchPin) == LOW)
    {
        Launch_Count++;
    }
    else
    {
        Launch_Count = 0;
    }

    if (Launch_Count >= POSITION_CHANGING_THRESHOLD)
    {
        B3M_setPosition(0x01, 4500, 10);
        Launch_Count = 0;
        Position = 2;
        delay(50);
    }

    //-------------------------------------------------//

    if (Position == 2 && digitalRead(WaitingPin) == LOW)
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
        B3M_setPosition(0x01, -4500, 10);
        Position = 1;
        delay(50);
    }

    // Serial.print(rawData);
    // Serial.print(", ");

    MAX31855_errornotification(); // MAX31855 のエラーをお知らせ
    FILLING_confirmation();

    sample[7] = sample[7] + 1;

    if (sample[7] == 100)
    {
        sample[7] = 0;
        sample[6] = sample[6] + 1;

        if (sample[6] == 100)
        {
            sample[6] = 0;
            sample[5] = sample[5] + 1;
        }
    }
    // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    CAN.sendMsgBuf(0x00, 0, 8, sample);
    delay(100);
    Serial.println("CAN BUS sendMsgBuf OK!!");

    // rawData = myMAX31855.readRawData();

    // Serial.print(Launch_Count);
    // Serial.print(",");
    // Serial.println(Waiting_Count);


    /*RS405CB テスト用*/
    delay(100);
    Move(1, 900, 50); //ID =1, GoalPosition = 90.0deg(900), Time = 0.5sec(50)
    delay(700);
    Move(1, -900, 100); //ID =1, GoalPosition = -90.0deg(-900), Time = 1.0sec(100)
    delay(1100);
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
        Serial.print(txCmd[i], HEX);
        Serial.print(" ");
    }
    Serial.println("");
    txCmd[7] = (byte)(txCmd[7]); // CHECKSUM

    // flag = B3M.synchronize(txCmd, sizeof txCmd, rxCmd, sizeof rxCmd);
    flag = B3M.synchronize(txCmd, 8, rxCmd, 5);

    if (flag == false)
    {
        Serial.println("ERROR");
        return -1;
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
    }
    reData = rxCmd[7];
    for (int i = 0; i < 7; i++)
    {
        Serial.print(reData, HEX);
        Serial.print(" ");
    }
    Serial.println("");

    return reData;
}

void Toque(unsigned char ID, unsigned char data)
{
    unsigned char TxData[9];
    unsigned char CheckSum = 0;

//パケットデータ生成
    TxData[0] = 0xFA;  //HEADER
    TxData[1] = 0xAF;  //HEADER
    TxData[2] = ID;    //ID
    TxData[3] = 0x00;  //FLAGS
    TxData[4] = 0x24;  //ADDRESS
    TxData[5] = 0x01;  //LENGTH
    TxData[6] = 0x01;  //COUNT
    TxData[7] = data;  //DATA

    for (int i = 2; i<=7;i++)
    {
        CheckSum = CheckSum ^ TxData[i];
    }
    TxData[8] = CheckSum;

    digitalWrite(REDE, HIGH);
    for (int i = 0; i<=8;i++)
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

//パケットデータ生成
    TxData[0] = 0xFA; //HEADER
    TxData[1] = 0xAF; //HEADER
    TxData[2] = ID;   //ID
    TxData[3] = 0x00; //FLAGS
    TxData[4] = 0x1E; //ADDRESS
    TxData[5] = 0x04; //LENGTH
    TxData[6] = 0x01; //COUNT
    //Angle
    TxData[7] = (unsigned char)0x00FF & Angle;
    TxData[8] = (unsigned char)0x00FF &(Angle >> 8);
    //Speed
    TxData[9] = (unsigned char)0x00FF & Speed;
    TxData[10] = (unsigned char)0x00FF & (Speed >> 8);
    //チェックサム計算
    for (int i = 2; i<=10;i++)
    {
        CheckSum = CheckSum ^ TxData[i];
    } 
    TxData[11] = CheckSum;

    //パケットデータ送信
    digitalWrite(REDE, HIGH);
    for (int i = 0; i<=11; i++)
    {
        Serial1.write(TxData[i]);
    }
    Serial1.flush();

    digitalWrite(REDE, LOW);
}

void B3M_initialize()
{
    B3M_writeCommand(0x01, 0x02, 0x28); // 動作モード：Free
    LED_blink();
    delay(500);
    B3M_writeCommand(0x01, 0x02, 0x28); // 位置制御モードに設定
    LED_blink();
    delay(500);
    B3M_writeCommand(0x01, 0x01, 0x29); // 起動生成タイプ：Even
    LED_blink();
    delay(500);
    B3M_writeCommand(0x01, 0x00, 0x5c); // ゲインプリセット：No.0
    LED_blink();
    delay(500);
    B3M_writeCommand(0x01, 0x00, 0x28); // 動作モード：Normal
    LED_blink();
    delay(1000);
}

void MAX31855_initialize()
{
    myMAX31855.begin();
    while (myMAX31855.getChipID() != MAX31855_ID)
    {
        Serial.println(F("MAX31855 error"));
        delay(5000);
    }
}

void MAX31855_errornotification()
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
    }
}

void FILLING_confirmation()
{
    // Serial.print(myMAX31855.getTemperature(rawData));
    if (myMAX31855.getTemperature(rawData) >= 30.00)
    {
        Serial.println("COMPLETED");
    }
}

void LED_blink()
{
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
}


void SIGNAL_initialize()
{
    pinMode(WaitingPin, INPUT_PULLUP);
    pinMode(LaunchPin, INPUT_PULLUP);
}

void CAN_sendtemperature(uint32_t id, float value)
{
    converter.value = value;

}