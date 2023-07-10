#include "CANSTM.hpp"


/// @brief CANコントローラ内蔵STM32用クラス
void CANSTM::begin() {
  //ボーレートは250kbpsで固定
  ACAN_STM32_Settings settings(250000);
  settings.mModuleMode = ACAN_STM32_Settings::NORMAL;
  settings.mTransmitPriority = ACAN_STM32_Settings::BY_REQUEST_ORDER;
  can.begin(settings);
}


/// @brief CANを初期化して起動する
bool CANSTM::available() {
  bool isAvailable = can.available0();

  // 受信しているなら内部変数にデータを保存
  if (isAvailable) {
    CANMessage message;
    can.receive0(message);

    _latestLabel = message.id;
    memcpy(_latestData, message.data, 8);
  }

  return isAvailable;
}


/// @brief 最後に受信したCANパケットのラベルを返す
/// @return 最後に受信したCANパケットのラベル
CANSTM::Label CANSTM::getLatestMessageLabel() {
  return static_cast<CANSTM::Label>(_latestLabel);
}


/// @brief システムステータスを送信する
/// @param mode フライトモード
/// @param camera true: ON, false: OFF
/// @param sn3 true: ON, false: OFF
// TODO 摘出した列挙型に変更
void CANSTM::sendSystemStatus(uint8_t mode, bool camera, bool sn3) {
  CANMessage message;
  message.id = static_cast<uint32_t>(Label::SYSTEM_STATUS);
  message.len = 3;

  message.data[0] = mode;
  message.data[1] = camera;
  message.data[2] = sn3;

  can.tryToSendReturnStatus(message);
}


/// @brief イベントを送信する
/// @param publisher どのモジュールがイベントを発行したか
/// @param eventCode イベントの種類
/// @param timestamp イベントを発行した時刻 (不要ならデフォルトで0)
void CANSTM::sendEvent(Publisher publisher, EventCode eventCode, uint32_t timestamp) {
  CANMessage message;
  message.id = static_cast<uint32_t>(Label::EVENT);
  message.len = 6;

  message.data[0] = static_cast<uint32_t>(publisher);
  message.data[1] = static_cast<uint32_t>(eventCode);
  memcpy(message.data + 2, &timestamp, 4);

  can.tryToSendReturnStatus(message);
}


/// @brief エラーを送信する
/// @param publisher どのモジュールがエラーを発行したか
/// @param errorCode エラーの種類
/// @param errorReason エラーの理由
/// @param timestamp イベントを発行した時刻 (不要ならデフォルトで0)
void CANSTM::sendError(Publisher publisher, ErrorCode errorCode, ErrorReason errorReason, uint32_t timestamp) {
  CANMessage message;
  message.id = static_cast<uint32_t>(Label::ERROR);
  message.len = 7;

  message.data[0] = static_cast<uint32_t>(publisher);
  message.data[1] = static_cast<uint32_t>(errorCode);
  message.data[1] = static_cast<uint32_t>(errorReason);
  memcpy(message.data + 3, &timestamp, 4);

  can.tryToSendReturnStatus(message);
}


/// @brief 軌道情報を送信する
/// @param isFalling true: 落下中, false: 落下中でない
void CANSTM::sendTrajectoryData(bool isFalling) {
  CANMessage message;
  message.id = static_cast<uint32_t>(Label::TRAJECTORY_DATA);
  message.len = 1;

  message.data[0] = isFalling;

  can.tryToSendReturnStatus(message);
}


/// @brief スカラー値を送信する
/// @param label データの種類
/// @param value 値
void CANSTM::sendScalar(Label label, float value) {
  CANMessage message;
  message.id = static_cast<uint32_t>(label);
  message.len = 4;

  memcpy(message.data, &value, 4);

  can.tryToSendReturnStatus(message);
}


/// @brief ベクトル値を送信する
/// @param label データの種類
/// @param axis 軸
/// @param value 値
void CANSTM::sendVector(Label label, Axis axis, float value) {
  CANMessage message;
  message.id = static_cast<uint32_t>(label);
  message.len = 5;

  message.data[0] = static_cast<uint8_t>(axis);
  memcpy(message.data + 1, &value, 4);

  can.tryToSendReturnStatus(message);
}


/// @brief 3次元のベクトル値を送信する
/// @param label データの種類
/// @param xValue x軸の値
/// @param yValue y軸の値
/// @param zValue z軸の値
void CANSTM::sendVector3D(Label label, float xValue, float yValue, float zValue) {
  //3軸分sendVectorを実行してるだけ
  sendVector(label, Axis::X, xValue);
  sendVector(label, Axis::Y, yValue);
  sendVector(label, Axis::Z, zValue);
}


/// @brief システムステータスを受信する
/// @param mode フライトモード
/// @param camera true: ON, false: OFF
/// @param sn3 true: ON, false: OFF
// TODO 摘出した列挙型に変更
// TODO receiveSystemStatusに改名
void CANSTM::receiveStatus(uint8_t* mode, bool* camera, bool* sn3) {
  *mode = _latestData[0];
  *camera = _latestData[1];
  *sn3 = _latestData[2];
}


/// @brief スカラー値を受信する
/// @param value 値のポインタ
void CANSTM::receiveScalar(float* value) {
  memcpy(value, _latestData, 4);
}


/// @brief 3次元のベクトル値を受信する
/// @param xValue x軸の値のポインタ
/// @param yValue y軸の値のポインタ
/// @param zValue z軸の値のポインタ
// TODO 3Dに改名
void CANSTM::receiveVector(float* xValue, float* yValue, float* zValue) {
  float value;
  memcpy(&value, _latestData + 1, 4);

  uint8_t axis = _latestData[0];
  // 引数では3軸分のポインタをもらうが、データの軸によって代入するのは1軸だけ
  switch (axis) {
  case static_cast<uint8_t>(Axis::X):
    *xValue = value;
    break;
  case static_cast<uint8_t>(Axis::Y):
    *yValue = value;
    break;
  case static_cast<uint8_t>(Axis::Z):
    *zValue = value;
    break;
  }
}


/// @brief 参照気圧セットを受信する
/// @param referencePressure 参照気圧のポインタ
// TODO receiveSetReferencePressureに改名
void CANSTM::receiveSetReferencePressureCommand(float* referencePressure) {
  memcpy(referencePressure, _latestData, 4);
}


/// @brief 軌道情報の受信する
/// @param isFalling true: 落下中, false: 落下中でない
void CANSTM::receiveTrajectoryData(bool* isFalling) {
  *isFalling = _latestData[0];
}