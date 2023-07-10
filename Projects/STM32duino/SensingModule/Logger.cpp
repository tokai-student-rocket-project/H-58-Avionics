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
}


/// @brief 全てのデータをシリアルに出力する ブロッキング処理で時間がかかる
void Logger::dump() {
  _fram0->dump();
  _fram1->dump();
}


/// @brief 全てに0を書き込み初期化する ブロッキング処理で時間がかかる
void Logger::clear() {
  _fram0->clear();
  _fram1->clear();
}


/// @brief ログを保存する
void Logger::log(
  uint32_t millis,
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
    0xAA, millis,
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

  // FRAMの2個分の容量を超えたら何もしない (容量オーバー)
  // TODO 例外処理
  if (_offset + size >= FRAM::LENGTH * 2) {
    return;
  }

  // FRAMの1個分の容量を超えたらFRAM1に書き込み
  // 超えていなければFRAM0に書き込み
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