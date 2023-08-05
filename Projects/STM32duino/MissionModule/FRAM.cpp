#include "FRAM.hpp"


/// @brief コンストラクタ
/// @param cs SPIのチップセレクト
FRAM::FRAM(uint32_t cs) {
  // 速度は20MHz あとはデータシート参照
  _setting = SPISettings(20000000, MSBFIRST, SPI_MODE0);
  _cs = cs;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
}


/// @brief ライトイネーブルをONにする これをしないと書き込めない
void FRAM::setWriteEnable() {
  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);

  SPI.transfer(WREN);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}


/// @brief FRAMのステータスを返す
/// @param buffer データ長1のバッファ
void FRAM::getStatus(uint8_t* buffer) {
  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);

  SPI.transfer(RDSR);
  // 引数の0xFFに意味はない
  buffer[0] = SPI.transfer(0xFF);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}


/// @brief FRAMのIdを返す
/// @param buffer データ長3のバッファ
void FRAM::getId(uint8_t* buffer) {
  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);

  SPI.transfer(RDID);
  // 引数の0xFFに意味はない
  buffer[0] = SPI.transfer(0xFF);
  buffer[1] = SPI.transfer(0xFF);
  buffer[2] = SPI.transfer(0xFF);
  buffer[3] = SPI.transfer(0xFF);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}


/// @brief データを読み出す
/// @param address アドレス
/// @return データ
uint8_t FRAM::read(uint32_t address) {
  // アドレスは24bitなので3x8bitに分ける
  uint8_t addressPart[3];
  memcpy(addressPart, &address, 3);

  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);

  SPI.transfer(READ);
  SPI.transfer(addressPart[2]);
  SPI.transfer(addressPart[1]);
  SPI.transfer(addressPart[0]);
  // 引数の0xFFに意味はない
  uint8_t data = SPI.transfer(0xFF);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  return data;
}


/// @brief データを書き込む
/// @param address アドレス
/// @param data データ
void FRAM::write(uint32_t address, uint8_t data) {
  // アドレスは24bitなので3x8bitに分ける
  uint8_t addressPart[3];
  memcpy(addressPart, &address, 3);

  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);

  SPI.transfer(WRITE);
  SPI.transfer(addressPart[2]);
  SPI.transfer(addressPart[1]);
  SPI.transfer(addressPart[0]);
  SPI.transfer(data);

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}


/// @brief 可変長のデータを書き込む
/// @param address アドレス
/// @param data データ
/// @param size データ長
void FRAM::write(uint32_t address, const uint8_t* data, uint32_t size) {
  // アドレスは24bitなので3x8bitに分ける
  uint8_t addressPart[3];
  memcpy(addressPart, &address, 3);

  SPI.beginTransaction(_setting);
  digitalWrite(_cs, LOW);

  SPI.transfer(WRITE);
  SPI.transfer(addressPart[2]);
  SPI.transfer(addressPart[1]);
  SPI.transfer(addressPart[0]);

  // データが複数のときは連続で書き込める
  for (uint32_t i = 0; i < size; i++) {
    SPI.transfer(data[i]);
  }

  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}


/// @brief 全てに0を書き込み初期化する ブロッキング処理で時間がかかる
void FRAM::clear() {
  for (uint32_t address = 0; address < LENGTH; address++) {
    write(address, 0x00);
  }
}


/// @brief 全てのデータをシリアルに出力する ブロッキング処理で時間がかかる
void FRAM::dump() {
  for (size_t address = 0; address < LENGTH; address++) {
    uint8_t data = read(address);

    Serial.print(data, HEX);

    // ストッピングビットの0だったら改行
    // それ以外はスペースを開けて続ける
    if (data == 0) {
      Serial.println();
    }
    else {
      Serial.print(" ");
    }
  }
}