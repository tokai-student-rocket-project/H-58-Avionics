#pragma once


#include <Arduino.h>
#include <mcp2515_can.h>
#include "Var.hpp"


/// @brief MCP2515 CANコントローラ用クラス
class CANMCP {
public:
  /// @brief CANのラベルを列挙型で定義しておく
  enum class Label : uint32_t {
    ORIENTATION,
    LINEAR_ACCELERATION,
    ALTITUDE,
    OUTSIDE_TEMPERATURE,
    VOLTAGE_SUPPLY,
    VOLTAGE_BATTERY,
    VOLTAGE_POOL,
    SYSTEM_STATUS,
    EVENT,
    ERROR,
    SET_REFERENCE_PRESSURE_COMMAND,
    TRAJECTORY_DATA,
    SENSING_STATUS,
    FLIGHT_MODE_ON_COMMAND,
    CLIMB_RATE,
    CURRENT_POSITION = 0x103,
    CURRENT_DESIRED_POSITION,
    CURRENT_VELOCITY,
    MCU_TEMPERATURE,
    MOTOR_TEMPERATURE,
    CURRENT,
    INPUT_VOLTAGE
  };

  /// @brief イベントとエラーを発行するモジュールを列挙型で定義しておく
  enum class Publisher : uint8_t {
    SENSING_MODULE,
    FLIGHT_MODULE,
    MISSION_MODULE,
    AIR_DATA_COMMUNICATION_MODULE,
    SYSTEM_DATA_COMMUNICATION_MODULE
  };

  /// @brief イベントを列挙型で定義しておく
  enum class EventCode : uint8_t {
    SETUP,
    RESET,
    FLIGHT_MODE_ON,
    IGNITION,
    BURNOUT,
    APOGEE,
    SEPARATE,
    LAND,
    FLIGHT_MODE_OFF,
    FORCE_SEPARATE,
    REFERENCE_PRESSURE_UPDATED
  };

  /// @brief エラーを列挙型で定義しておく
  enum class ErrorCode : uint8_t {
    COMMAND_RECEIVE_FAILED,
    LOGGER_FAILURE
  };

  /// @brief エラーの理由を列挙型で定義しておく
  enum class ErrorReason : uint8_t {
    INVALID_KEY,
    INVALID_SD
  };


  /// @brief コンストラクタ
  /// @param cs SPIのチップセレクト
  CANMCP(uint8_t cs);

  /// @brief CANを初期化して起動する
  void begin();


  /// @brief CANパケットを受信しているかを返す
  /// @return true: 受信している, false: 受信していない
  bool available();

  /// @brief 最後に受信したCANパケットのラベルを返す
  /// @return 最後に受信したCANパケットのラベル
  Label getLatestLabel();


  /// @brief イベントを送信する
  /// @param publisher どのモジュールがイベントを発行したか
  /// @param eventCode イベントの種類
  /// @param timestamp イベントを発行した時刻 (不要ならデフォルトで0)
  void sendEvent(Publisher publisher, EventCode eventCode, uint32_t timestamp = 0);

  /// @brief エラーを送信する
  /// @param publisher どのモジュールがエラーを発行したか
  /// @param errorCode エラーの種類
  /// @param errorReason エラーの理由
  /// @param timestamp イベントを発行した時刻 (不要ならデフォルトで0)
  void sendError(Publisher publisher, ErrorCode errorCode, ErrorReason errorReason, uint32_t timestamp = 0);

  /// @brief 参照気圧セットを送信する
  /// @param referencePressure 参照気圧
  void sendSetReferencePressure(float referencePressure);

  /// @brief 参照気圧セットを送信する
  /// @param referencePressure 参照気圧
  void sendFlightModeOn();


  /// @brief システムステータスを受信する
  /// @param flightMode フライトモード
  /// @param cameraState カメラの状態
  /// @param sn3State 不知火3の状態
  /// @param doLogging ログ保存するか
  void receiveSystemStatus(Var::FlightMode* flightMode, Var::State* cameraState, Var::State* sn3State, bool* doLogging);

  /// @brief 計測ステータスを受信する
  /// @param referencePressure 参照気圧 [hPa]
  /// @param isSystemCalibrated BNO055システムのキャリブレーションが完了しているか
  /// @param isGyroscopeCalibrated BNO055角加速度計のキャリブレーションが完了しているか
  /// @param isAccelerometerCalibrated BNO055加速度計のキャリブレーションが完了しているか
  /// @param isMagnetometerCalibrated BNO055地磁気計のキャリブレーションが完了しているか
  void receiveSensingStatus(float* referencePressure, bool* isSystemCalibrated, bool* isGyroscopeCalibrated, bool* isAccelerometerCalibrated, bool* isMagnetometerCalibrated);

  /// @brief スカラー値を受信する
  /// @param value 値のポインタ
  void receiveScalar(float* value);

  /// @brief 3次元のベクトル値を受信する
  /// @param xValue x軸の値のポインタ
  /// @param yValue y軸の値のポインタ
  /// @param zValue z軸の値のポインタ
  void receiveVector(float* xValue, float* yValue, float* zValue);

  /// @brief イベントを受診する
  /// @param publisher どのモジュールがイベントを発行したか
  /// @param eventCode イベントの種類
  /// @param timestamp イベントを発行した時刻 (不要ならデフォルトで0)
  void receiveEvent(Publisher* publisher, EventCode* eventCode, uint32_t* timestamp);

  /// @brief エラーを受信する
  /// @param publisher どのモジュールがエラーを発行したか
  /// @param errorCode エラーの種類
  /// @param errorReason エラーの理由
  /// @param timestamp イベントを発行した時刻 (不要ならデフォルトで0)
  void receiveError(Publisher* publisher, ErrorCode* errorCode, ErrorReason* errorReason, uint32_t* timestamp);

  void receiveServo(float* value);

private:
  mcp2515_can* _can;

  uint32_t _latestLabel;
  uint8_t _latestData[8];
};