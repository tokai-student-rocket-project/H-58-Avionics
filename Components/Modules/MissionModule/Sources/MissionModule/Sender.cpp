#include "Sender.hpp"


/// @brief コンストラクタ
/// @param csFram0 1つ目のFRAMのチップセレクト
/// @param csFram1 2つ目のFRAMのチップセレクト
Sender::Sender(uint32_t csFram0, uint32_t csFram1) {
  _fram0 = new FRAM(csFram0);
  _fram1 = new FRAM(csFram1);
}


/// @brief 読み出し位置を最初に戻す
void Sender::reset() {
  _offset = 0;
}


/// @brief ログを送信する
void Sender::send(uint32_t count) {
  uint32_t tempSize = 24 * 25;
  uint32_t offset = count * tempSize;

  uint8_t data[24] = { 0 };
  uint32_t size = 0;

  // FRAMの2個分の容量を超えたら何もしない (容量オーバー)
  if (offset + tempSize >= FRAM::LENGTH * 2) return;

  if (offset + tempSize >= FRAM::LENGTH) {
    uint32_t writeAddress = offset - FRAM::LENGTH;
    for (uint32_t i = 0; i < 24; i++) {
      uint8_t iData = _fram1->read(writeAddress + i);
      data[i] = iData;

      if (data[1] == 0xAA && data[i] == 0x00) {
        size = i + 1;
        break;
      };
    }
  }
  else {
    uint32_t writeAddress = offset;
    for (uint32_t i = 0; i < 24; i++) {
      uint8_t iData = _fram0->read(writeAddress + i);
      data[i] = iData;

      if (data[1] == 0xAA && data[i] == 0x00) {
        size = i + 1;
        break;
      };
    }
  }

  LoRa.beginPacket();
  LoRa.write(data, size);
  LoRa.endPacket();

  _offset += tempSize;
}


uint32_t Sender::getOffset() {
  return _offset;
}