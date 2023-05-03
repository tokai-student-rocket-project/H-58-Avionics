HardwareSerial Serial1(PA_10, PA_9);

void setup()
{
    Serial1.begin(115200, SERIAL_8E1);
}

void loop()
{
    move_b3m(0, )
}

void move_b3m(byte id)
{
    byte move_tx[9] = {0x09, 0x04, 0x00, 0x00, 0x05, 0x00, 0x2A, 0x01, 0x3D};
    byte power_tx[9] = {0x09, 0x04, 0x00, 0x00, 0x00, 0x00, 0x28, 0x01, 0x36};
    int sum;

    move_tx[3] = id;
    power_rx[3] = id;

    power_tx[4] = 0x00;
    sum = 0;
    for (byte i = 0; i <= 7; i++)
    sum+= power_tx[i];

    power_tx[8] = (byte)(sum & 0x00FF);
    Serial.write(power_tx, 9);
    delayMicroseconds(1500);
    move_tx[4] = (byte)()
}