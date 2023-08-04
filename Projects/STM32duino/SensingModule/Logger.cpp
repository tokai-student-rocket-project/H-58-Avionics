#include "Logger.hpp"


/// @brief コンストラクタ
/// @param csFram0 1つ目のFRAMのチップセレクト
/// @param csFram1 2つ目のFRAMのチップセレクト
/// @param csSd SDのチップセレクト
Logger::Logger(uint32_t csFram0, uint32_t csFram1, uint32_t csSd) {
  _fram0 = new FRAM(csFram0);
  _fram1 = new FRAM(csFram1);
  _sd = new Sd(csSd);
}


/// @brief 書き込み位置を最初に戻す 元のデータは上書きされるので注意
void Logger::reset() {
  _offset = 0;
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


/// @brief ログ保存を開始する
/// @return true: 開始成功, false: 開始失敗
bool Logger::beginLogging(bool useSd) {
  // すでにログ保存がONの場合は何もしない
  // 多重リセット防止
  if (_isLogging) {
    return _isLogging;
  }

  _isLogging = true;

  // リセットして書き込み位置を最初に戻す
  reset();

  bool isSucceeded = false;
  if (useSd) {
    isSucceeded = _sd->beginLogging();
  }

  return isSucceeded;
}


/// @brief ログ保存を終了する
void Logger::endLogging() {
  _sd->endLogging();

  _isLogging = false;
}


/// @brief ログ保存の状態を返す
/// @return true: 保存中, false: 保存中でない
bool Logger::isLogging() {
  return _isLogging;
}


/// @brief ログを保存する
void Logger::log(
  uint32_t millis, uint8_t flightMode,
  float outsideTemperature, float pressure, float altitude, float climbIndex, bool isFalling,
  float acceleration_x, float acceleration_y, float acceleration_z,
  float gyroscope_x, float gyroscope_y, float gyroscope_z,
  float magnetometer_x, float magnetometer_y, float magnetometer_z,
  float orientation_x, float orientation_y, float orientation_z,
  float linear_acceleration_x, float linear_acceleration_y, float linear_acceleration_z,
  float gravity_x, float gravity_y, float gravity_z
) {
  // MessagePackでパケットを生成
  // ラベルは認識しやすいように0xAAにしている
  const auto& packet = MsgPacketizer::encode(
    0xAA, millis, flightMode,
    outsideTemperature, pressure, altitude, climbIndex, isFalling,
    acceleration_x, acceleration_y, acceleration_z,
    gyroscope_x, gyroscope_y, gyroscope_z,
    magnetometer_x, magnetometer_y, magnetometer_z,
    orientation_x, orientation_y, orientation_z,
    linear_acceleration_x, linear_acceleration_y, linear_acceleration_z,
    gravity_x, gravity_y, gravity_z
  );

  const uint8_t* data = packet.data.data();
  const uint32_t size = packet.data.size();

  // SDへの保存
  _sd->write(data, size);

  // FRAMの2個分の容量を超えたら何もしない (容量オーバー)
  if (_offset + size >= FRAM::LENGTH * 2) return;

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



}


/// @brief ロガーの使用率
/// @return FRAM全て合わせた使用率 パーセント
float Logger::getUsage() {
  return ((float)_offset / (float)(FRAM::LENGTH * 2)) * 100.0;
}