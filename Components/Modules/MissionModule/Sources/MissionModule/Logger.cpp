#include "Logger.hpp"


/// @brief コンストラクタ
/// @param csFram0 1つ目のFRAMのチップセレクト
/// @param csFram1 2つ目のFRAMのチップセレクト
Logger::Logger(uint32_t csFram0, uint32_t csFram1) {
  _fram0 = new FRAM(csFram0);
  _fram1 = new FRAM(csFram1);
}


/// @brief 書き込み位置を最初に戻す 元のデータは上書きされるので注意
void Logger::reset() {
  _offset = 0;

  uint8_t buffer[4];
  _fram0->getId(buffer);
  Serial.println(buffer[0], BIN);
  Serial.println(buffer[1], BIN);
  Serial.println(buffer[2], BIN);
  Serial.println(buffer[3], BIN);

  _fram0->setWriteEnable();
  _fram0->write(0, 0xCC);
  Serial.println(_fram0->read(0), HEX);
}


/// @brief 全てのデータをシリアルに出力する ブロッキング処理で時間がかかる
void Logger::dump() {
  _fram0->dump();
  _fram1->dump();
}


/// @brief 全てに0を書き込み初期化する ブロッキング処理で時間がかかる
void Logger::clear() {
  _fram0->setWriteEnable();
  _fram0->clear();
  _fram1->setWriteEnable();
  _fram1->clear();
}


/// @brief ログを保存する
uint32_t Logger::log(
  uint32_t millis, uint8_t flightMode,
  float x, float y, float z
) {
  // MessagePackでパケットを生成
  // ラベルは認識しやすいように0xAAにしている
  const auto& packet = MsgPacketizer::encode(
    0xAA, millis, flightMode,
    x, y, z
  );

  const uint8_t* packetData = packet.data.data();
  const uint32_t packetSize = packet.data.size();

  uint8_t data[32];
  uint32_t size = 32;

  memcpy(data, packetData, packetSize);

  // for (size_t i = 0; i < size; i++) {
  //   Serial.print(data[i], HEX);
  //   Serial.print(" ");
  // }

  // Serial.println(size);


  // FRAMの2個分の容量を超えたら何もしない (容量オーバー)
  if (_offset + size >= FRAM::LENGTH * 2) return size;

  // Serial.println(_offset);

  if (_offset + size >= FRAM::LENGTH) {
    uint32_t writeAddress = _offset - FRAM::LENGTH;
    _fram1->setWriteEnable();
    _fram1->write(writeAddress, data, size);
  }
  else {
    uint32_t writeAddress = _offset;
    _fram0->setWriteEnable();
    _fram0->write(writeAddress, data, size);
  }

  _offset += size;
  return size;
}


uint32_t Logger::getOffset() {
  return _offset;
}


/// @brief ロガーの使用率
/// @return FRAM全て合わせた使用率 パーセント
float Logger::getUsage() {
  return ((float)_offset / (float)(FRAM::LENGTH * 2)) * 100.0;
}