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


/// @brief ログ保存を開始する
/// @return true: 開始成功, false: 開始失敗
bool Logger::beginLogging() {
  // すでにログ保存がONの場合は何もしない
  // 多重リセット防止
  if (_isLogging) {
    return _isLogging;
  }

  _isLogging = true;

  // リセットして書き込み位置を最初に戻す
  reset();

  return _isLogging;
}


/// @brief ログ保存を終了する
void Logger::endLogging() {
  _isLogging = false;
}


/// @brief ログ保存の状態を返す
/// @return true: 保存中, false: 保存中でない
bool Logger::isLogging() {
  return _isLogging;
}


/// @brief ログを保存する
void Logger::log(
  uint32_t millis, uint16_t flightTime,
  uint8_t flightMode, bool cameraState, bool sn3State, bool doLogging,
  bool isFalling, bool flightPinState, bool resetPinState,
  float supplyVoltage, float batteryVoltage, float poolVoltage,
  bool isLaunchMode,
  float motorTemperature, float mcuTemperature, float current, float inputVoltage,
  float currentPosition, float currentDesiredPosition, float currentVelocity,
  uint32_t performanceMillis, float performanceTaskRate
) {
  // MessagePackでパケットを生成
  // ラベルは認識しやすいように0xAAにしている
  const auto& packet = MsgPacketizer::encode(
    0xAA, millis, flightTime,
    flightMode, cameraState, sn3State, doLogging,
    isFalling, flightPinState, resetPinState,
    supplyVoltage, batteryVoltage, poolVoltage,
    isLaunchMode,
    motorTemperature, mcuTemperature, current, inputVoltage,
    currentPosition, currentDesiredPosition, currentVelocity,
    performanceMillis, performanceTaskRate
  );

  const uint8_t* data = packet.data.data();
  const uint32_t size = packet.data.size();

  // FRAMの容量を超えたら何もしない (容量オーバー)
  if (_offset + size >= FRAM::LENGTH) return;

  _fram->setWriteEnable();
  _fram->write(_offset, data, size);

  _offset += size;
}


/// @brief ロガーの使用率
/// @return FRAM全て合わせた使用率 パーセント
float Logger::getUsage() {
  return ((float)_offset / (float)FRAM::LENGTH) * 100.0;
}