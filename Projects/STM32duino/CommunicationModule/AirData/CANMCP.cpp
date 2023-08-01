#include "CANMCP.hpp"


/// @brief コンストラクタ
/// @param cs SPIのチップセレクト
CANMCP::CANMCP(uint8_t cs) {
  _can = new mcp2515_can(cs);
}


/// @brief CANを初期化して起動する
void CANMCP::begin() {
  _can->begin(CAN_1000KBPS, MCP_8MHz);
}


/// @brief CANパケットを受信しているかを返す
/// @return true: 受信している, false: 受信していない
bool CANMCP::available() {
  bool isAvailable = _can->checkReceive() == CAN_MSGAVAIL;

  // もし受信しているなら、最新のデータとラベルを保存しておく
  if (isAvailable) {
    uint8_t len = 0;
    _can->readMsgBuf(&len, _latestData);
    _latestLabel = _can->getCanId();
  }

  return isAvailable;
}


/// @brief 最後に受信したCANパケットのラベルを返す
/// @return 最後に受信したCANパケットのラベル
CANMCP::Label CANMCP::getLatestLabel() {
  return static_cast<CANMCP::Label>(_latestLabel);
}


/// @brief イベントを送信する
/// @param publisher どのモジュールがイベントを発行したか
/// @param eventCode イベントの種類
/// @param timestamp イベントを発行した時刻 (不要ならデフォルトで0)
void CANMCP::sendEvent(Publisher publisher, EventCode eventCode, uint32_t timestamp) {
  uint8_t data[6];
  data[0] = static_cast<uint8_t>(publisher);
  data[1] = static_cast<uint8_t>(eventCode);
  memcpy(data + 2, &timestamp, 4);

  _can->sendMsgBuf(static_cast<uint32_t>(Label::EVENT), 0, 6, data);
}


/// @brief エラーを送信する
/// @param publisher どのモジュールがエラーを発行したか
/// @param errorCode エラーの種類
/// @param errorReason エラーの理由
/// @param timestamp イベントを発行した時刻 (不要ならデフォルトで0)
void CANMCP::sendError(Publisher publisher, ErrorCode errorCode, ErrorReason errorReason, uint32_t timestamp) {
  uint8_t data[7];
  data[0] = static_cast<uint8_t>(publisher);
  data[1] = static_cast<uint8_t>(errorCode);
  data[2] = static_cast<uint8_t>(errorReason);
  memcpy(data + 3, &timestamp, 4);

  _can->sendMsgBuf(static_cast<uint32_t>(Label::ERROR), 0, 7, data);
}


/// @brief 参照気圧セットを送信する
/// @param referencePressure 参照気圧
void CANMCP::sendSetReferencePressure(float payload) {
  uint8_t data[4];
  memcpy(data, &payload, 4);

  _can->sendMsgBuf(static_cast<uint32_t>(Label::SET_REFERENCE_PRESSURE_COMMAND), 0, 4, data);
}


/// @brief 参照気圧セットを送信する
void CANMCP::sendFlightModeOn() {
  uint8_t data[0];
  _can->sendMsgBuf(static_cast<uint32_t>(Label::FLIGHT_MODE_ON_COMMAND), 0, 0, data);
}


/// @brief リセットを送信する
void CANMCP::sendReset() {
  uint8_t data[0];
  _can->sendMsgBuf(static_cast<uint32_t>(Label::RESET_COMMAND), 0, 0, data);
}


/// @brief システムステータスを受信する
/// @param flightMode フライトモード
/// @param cameraState カメラの状態
/// @param sn3State 不知火3の状態
/// @param doLogging ログ保存するか
void CANMCP::receiveSystemStatus(Var::FlightMode* flightMode, Var::State* cameraState, Var::State* sn3State, bool* doLogging, uint32_t* flightTime) {
  *flightMode = static_cast<Var::FlightMode>(_latestData[0]);
  *cameraState = static_cast<Var::State>(_latestData[1]);
  *sn3State = static_cast<Var::State>(_latestData[2]);
  *doLogging = _latestData[3];
  memcpy(flightTime, _latestData + 4, 4);
}


/// @brief 計測ステータスを受信する
/// @param referencePressure 参照気圧 [hPa]
/// @param isSystemCalibrated BNO055システムのキャリブレーションが完了しているか
/// @param isGyroscopeCalibrated BNO055角加速度計のキャリブレーションが完了しているか
/// @param isAccelerometerCalibrated BNO055加速度計のキャリブレーションが完了しているか
/// @param isMagnetometerCalibrated BNO055地磁気計のキャリブレーションが完了しているか
void CANMCP::receiveSensingStatus(float* referencePressure, bool* isSystemCalibrated, bool* isGyroscopeCalibrated, bool* isAccelerometerCalibrated, bool* isMagnetometerCalibrated) {
  memcpy(referencePressure, _latestData, 4);
  *isSystemCalibrated = _latestData[4];
  *isGyroscopeCalibrated = _latestData[5];
  *isAccelerometerCalibrated = _latestData[6];
  *isMagnetometerCalibrated = _latestData[7];
}


/// @brief スカラー値を受信する
/// @param value 値のポインタ
void CANMCP::receiveScalar(float* value) {
  memcpy(value, _latestData, 4);
}


/// @brief double型のスカラー値を受信する
/// @param value 値のポインタ
void CANMCP::receiveScalaDouble(float* value) {
  double valueDouble;
  memcpy(&valueDouble, _latestData, 8);

  *value = (float)valueDouble;
}


/// @brief 3次元のベクトル値を受信する
/// @param xValue x軸の値のポインタ
/// @param yValue y軸の値のポインタ
/// @param zValue z軸の値のポインタ
void CANMCP::receiveVector(float* xValue, float* yValue, float* zValue) {
  float value;
  memcpy(&value, _latestData + 1, 4);

  uint8_t axis = _latestData[0];
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


/// @brief イベントを受診する
/// @param publisher どのモジュールがイベントを発行したか
/// @param eventCode イベントの種類
/// @param timestamp イベントを発行した時刻 (不要ならデフォルトで0)
void CANMCP::receiveEvent(Publisher* publisher, EventCode* eventCode, uint32_t* timestamp) {
  *publisher = static_cast<CANMCP::Publisher>(_latestData[0]);
  *eventCode = static_cast<CANMCP::EventCode>(_latestData[1]);
  memcpy(timestamp, _latestData + 2, 4);
}


/// @brief エラーを受信する
/// @param publisher どのモジュールがエラーを発行したか
/// @param errorCode エラーの種類
/// @param errorReason エラーの理由
/// @param timestamp イベントを発行した時刻 (不要ならデフォルトで0)
void CANMCP::receiveError(Publisher* publisher, ErrorCode* errorCode, ErrorReason* errorReason, uint32_t* timestamp) {
  *publisher = static_cast<CANMCP::Publisher>(_latestData[0]);
  *errorCode = static_cast<CANMCP::ErrorCode>(_latestData[1]);
  *errorReason = static_cast<CANMCP::ErrorReason>(_latestData[2]);
  memcpy(timestamp, _latestData + 3, 4);
}


/// @brief バルブ情報を受信する
/// @param motorTemperature モーター温度 [degC]
/// @param mcuTemperature マイコン温度 [degC]
/// @param current 電流 [A]
/// @param inputVoltage 電圧 [V]
void CANMCP::receiveValveData1(float* motorTemperature, float* mcuTemperature, float* current, float* inputVoltage) {
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
void CANMCP::receiveValveData2(float* currentPosition, float* currentDesiredPosition, float* currentVelocity) {
  int16_t currentPositionRaw, currentDesiredPositionRaw, currentVelocityRaw;

  memcpy(&currentPositionRaw, _latestData, 2);
  memcpy(&currentDesiredPositionRaw, _latestData + 2, 2);
  memcpy(&currentVelocityRaw, _latestData + 4, 2);

  *currentPosition = (float)currentPositionRaw;
  *currentDesiredPosition = (float)currentDesiredPositionRaw;
  *currentVelocity = (float)currentVelocityRaw;
}


/// @brief バルブモードを受信する
/// @param isWaiting WAITINGモードか
void CANMCP::receiveValveMode(bool* isWaiting) {
  *isWaiting = _latestData[0];
}


/// @brief 電圧を受信する
/// @param supply 供給電圧 [V]
/// @param pool プール電圧 [V]
/// @param battery バッテリー電圧 [V]
void CANMCP::receiveVoltage(float* supply, float* pool, float* battery) {
  int16_t supplyInt, poolInt, batteryInt;
  memcpy(&supplyInt, _latestData + 0, 2);
  memcpy(&poolInt, _latestData + 2, 2);
  memcpy(&batteryInt, _latestData + 4, 2);

  *supply = (float)supplyInt / 100.0;
  *pool = (float)poolInt / 100.0;
  *battery = (float)batteryInt / 100.0;
}