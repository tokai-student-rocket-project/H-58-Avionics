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
uint32_t Sender::send() {
  const uint32_t size = 19 * 25;

  // // FRAMの2個分の容量を超えたら何もしない (容量オーバー)
  // if (_offset + size >= FRAM::LENGTH * 2) return size;

  // if (_offset + size >= FRAM::LENGTH) {
  //   uint32_t writeAddress = _offset - FRAM::LENGTH;
  //   _fram1->setWriteEnable();
  //   _fram1->write(writeAddress, data, size);
  // }
  // else {
  //   uint32_t writeAddress = _offset;
  //   _fram0->setWriteEnable();
  //   _fram0->write(writeAddress, data, size);
  // }

  _offset += size;
  return size;
}


uint32_t Sender::getOffset() {
  return _offset;
}