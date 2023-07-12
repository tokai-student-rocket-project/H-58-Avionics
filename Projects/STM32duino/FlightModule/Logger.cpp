#include "Logger.hpp"


/// @brief コンストラクタ
/// @param csFram FRAMのチップセレクト
Logger::Logger(uint32_t csFram) {
  _fram = new FRAM(csFram);
}


/// @brief 書き込み位置を最初に戻す 元のデータは上書きされるので注意
void Logger::reset() {
  _offset = 0;
}


/// @brief 全てのデータをシリアルに出力する ブロッキング処理で時間がかかる
void Logger::dump() {
  _fram->dump();
}


/// @brief 全てに0を書き込み初期化する ブロッキング処理で時間がかかる
void Logger::clear() {
  _fram->setWriteEnable();
  _fram->clear();
}


/// @brief ログを保存する
void Logger::log(
  uint32_t millis, uint32_t flightTime,
  uint8_t flightMode, bool cameraState, bool sn3State, bool doLogging,
  bool isFalling, bool flightPinState, bool resetPinState,
  float supplyVoltage, float batteryVoltage, float poolVoltage
) {
  // MessagePackでパケットを生成
  // ラベルは認識しやすいように0xAAにしている
  const auto& packet = MsgPacketizer::encode(
    0xAA, millis, flightTime,
    flightMode, cameraState, sn3State, doLogging,
    isFalling, flightPinState, resetPinState,
    supplyVoltage, batteryVoltage, poolVoltage
  );

  const uint8_t* data = packet.data.data();
  const uint32_t size = packet.data.size();

  // FRAMの容量を超えたら何もしない (容量オーバー)
  if (_offset + size < FRAM::LENGTH) {
    _fram->setWriteEnable();
    _fram->write(_offset, data, size);
  }

  _offset += size;
}