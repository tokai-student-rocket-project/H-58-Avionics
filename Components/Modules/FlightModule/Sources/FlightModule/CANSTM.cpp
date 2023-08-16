#include "CANSTM.hpp"


/// @brief CANコントローラ内蔵STM32用クラス
void CANSTM::begin() {
  //ボーレートは250kbpsで固定
  ACAN_STM32_Settings settings(1'000'000);
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
/// @param flightMode フライトモード
/// @param cameraState カメラの状態
/// @param sn3State 不知火3の状態
/// @param doLogging ログ保存するか
/// @param timestamp 飛翔時間
void CANSTM::sendSystemStatus(Var::FlightMode flightMode, Var::State cameraState, Var::State sn3State, bool doLogging, uint16_t flightTime, uint8_t loggerUsage) {
  CANMessage message;
  message.id = static_cast<uint32_t>(Label::SYSTEM_STATUS);
  message.len = 7;

  message.data[0] = static_cast<uint8_t>(flightMode);
  message.data[1] = static_cast<uint8_t>(cameraState);
  message.data[2] = static_cast<uint8_t>(sn3State);
  message.data[3] = doLogging;
  memcpy(message.data + 4, &flightTime, 2);
  message.data[6] = loggerUsage;

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


/// @brief 計測ステータスを送信する
/// @param referencePressure 参照気圧 [hPa]
/// @param isSystemCalibrated BNO055システムのキャリブレーションが完了しているか
/// @param loggerUsage ロガーの使用率
void CANSTM::sendSensingStatus(float referencePressure, bool isSystemCalibrated, uint8_t loggerUsage) {
  CANMessage message;
  message.id = static_cast<uint32_t>(Label::SENSING_STATUS);
  message.len = 6;

  memcpy(message.data, &referencePressure, 4);
  message.data[4] = isSystemCalibrated;
  message.data[5] = loggerUsage;

  can.tryToSendReturnStatus(message);
}


/// @brief 電圧を送信する
/// @param supply 供給電圧 [V]
/// @param pool プール電圧 [V]
/// @param battery バッテリー電圧 [V]
void CANSTM::sendVoltage(float supply, float pool, float battery) {
  CANMessage message;
  message.id = static_cast<uint32_t>(Label::VOLTAGE);
  message.len = 6;

  int16_t supplyInt = (int16_t)(supply * 100.0);
  int16_t poolInt = (int16_t)(pool * 100.0);
  int16_t batteryInt = (int16_t)(battery * 100.0);

  memcpy(message.data + 0, &supplyInt, 2);
  memcpy(message.data + 2, &poolInt, 2);
  memcpy(message.data + 4, &batteryInt, 2);

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
void CANSTM::sendVector(Label label, Var::Axis axis, float value) {
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
  sendVector(label, Var::Axis::X, xValue);
  sendVector(label, Var::Axis::Y, yValue);
  sendVector(label, Var::Axis::Z, zValue);
}


/// @brief システムステータスを受信する
/// @param flightMode フライトモード
/// @param cameraState カメラの状態
/// @param sn3State 不知火3の状態
/// @param doLogging ログ保存するか
/// @param flightTime 飛翔時間
void CANSTM::receiveSystemStatus(Var::FlightMode* flightMode, Var::State* cameraState, Var::State* sn3State, bool* doLogging, uint16_t* flightTime, uint8_t* loggerUsage) {
  *flightMode = static_cast<Var::FlightMode>(_latestData[0]);
  *cameraState = static_cast<Var::State>(_latestData[1]);
  *sn3State = static_cast<Var::State>(_latestData[2]);
  *doLogging = _latestData[3];
  memcpy(flightTime, _latestData + 4, 2);
  *loggerUsage = _latestData[6];
}


/// @brief スカラー値を受信する
/// @param value 値のポインタ
void CANSTM::receiveScalar(float* value) {
  memcpy(value, _latestData, 4);
}


/// @brief double型のスカラー値を受信する
/// @param value 値のポインタ
void CANSTM::receiveScalaDouble(float* value) {
  double valueDouble;
  memcpy(&valueDouble, _latestData, 8);

  *value = (float)valueDouble;
}


/// @brief 3次元のベクトル値を受信する
/// @param xValue x軸の値のポインタ
/// @param yValue y軸の値のポインタ
/// @param zValue z軸の値のポインタ
void CANSTM::receiveVector3D(float* xValue, float* yValue, float* zValue) {
  float value;
  memcpy(&value, _latestData + 1, 4);

  uint8_t axis = _latestData[0];
  // 引数では3軸分のポインタをもらうが、データの軸によって代入するのは1軸だけ
  switch (axis) {
  case static_cast<uint8_t>(Var::Axis::X):
    *xValue = value;
    break;
  case static_cast<uint8_t>(Var::Axis::Y):
    *yValue = value;
    break;
  case static_cast<uint8_t>(Var::Axis::Z):
    *zValue = value;
    break;
  }
}


/// @brief 参照気圧セットを受信する
/// @param referencePressure 参照気圧のポインタ
void CANSTM::receiveSetReferencePressure(float* referencePressure) {
  memcpy(referencePressure, _latestData, 4);
}


/// @brief 軌道情報の受信する
/// @param isFalling true: 落下中, false: 落下中でない
void CANSTM::receiveTrajectoryData(bool* isFalling) {
  *isFalling = _latestData[0];
}


/// @brief バルブ制御モードを受信する
/// @param isWaiting true: WAITING, false: LAUNCH
void CANSTM::receiveValveMode(bool* isWaiting) {
  *isWaiting = _latestData[0];
}


/// @brief バルブ情報を受信する
/// @param motorTemperature モーター温度 [degC]
/// @param mcuTemperature マイコン温度 [degC]
/// @param current 電流 [A]
/// @param inputVoltage 電圧 [V]
void CANSTM::receiveValveData1(float* motorTemperature, float* mcuTemperature, float* current, float* inputVoltage) {
  uint8_t motorTemperatureRaw, mcuTemperatureRaw, currentRaw, inputVoltageRaw;

  memcpy(&motorTemperatureRaw, _latestData, 1);
  memcpy(&mcuTemperatureRaw, _latestData + 1, 1);
  memcpy(&currentRaw, _latestData + 2, 1);
  memcpy(&inputVoltageRaw, _latestData + 3, 1);

  *motorTemperature = (float)motorTemperatureRaw;
  *mcuTemperature = (float)mcuTemperatureRaw;
  *current = (float)currentRaw / 100.0;
  *inputVoltage = (float)inputVoltageRaw / 10.0;
}

/// @brief バルブ情報を受信する
/// @param currentPosition 現在の角度 [deg]
/// @param currentDesiredPosition 目標の角度 [deg]
/// @param currentVelocity 角速度 [dps]
void CANSTM::receiveValveData2(float* currentPosition, float* currentDesiredPosition, float* currentVelocity) {
  int16_t currentPositionRaw, currentDesiredPositionRaw, currentVelocityRaw;

  memcpy(&currentPositionRaw, _latestData, 2);
  memcpy(&currentDesiredPositionRaw, _latestData + 2, 2);
  memcpy(&currentVelocityRaw, _latestData + 4, 2);

  *currentPosition = (float)currentPositionRaw;
  *currentDesiredPosition = (float)currentDesiredPositionRaw;
  *currentVelocity = (float)currentVelocityRaw;
}


/// @brief 機体間通信実証モジュールの性能情報を受信す
/// @param millis モジュール起動からの経過時間 [ms]
/// @param taskRate タスクの実行周波数 [Hz]
void CANSTM::receivePerformance(uint32_t* millis, float* taskRate) {
  memcpy(millis, _latestData, 4);
  memcpy(taskRate, _latestData + 4, 4);
}