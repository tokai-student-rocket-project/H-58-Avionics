#pragma once


#include <Arduino.h>
#include <ACAN_STM32.h>
#include "Var.hpp"


/// @brief CANコントローラ内蔵STM32用クラス
class CANSTM {
public:
  /// @brief CANのラベルを列挙型で定義しておく
  enum class Label : uint32_t {
    ORIENTATION,
    LINEAR_ACCELERATION,
    ALTITUDE,
    OUTSIDE_TEMPERATURE,
    VOLTAGE,
    SYSTEM_STATUS,
    EVENT,
    ERROR,
    SET_REFERENCE_PRESSURE,
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
    INPUT_VOLTAGE,
    VALVE_MODE,
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


  /// @brief CANを初期化して起動する
  void begin();


  /// @brief CANパケットを受信しているかを返す
  /// @return true: 受信している, false: 受信していない
  bool available();

  /// @brief 最後に受信したCANパケットのラベルを返す
  /// @return 最後に受信したCANパケットのラベル
  Label getLatestMessageLabel();


  /// @brief システムステータスを送信する
  /// @param flightMode フライトモード
  /// @param cameraState カメラの状態
  /// @param sn3State 不知火3の状態
  /// @param doLogging ログ保存するか
  void sendSystemStatus(Var::FlightMode flightMode, Var::State cameraState, Var::State sn3State, bool doLogging);

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

  /// @brief 軌道情報を送信する
  /// @param isFalling true: 落下中, false: 落下中でない
  void sendTrajectoryData(bool isFalling);

  /// @brief 計測ステータスを送信する
  /// @param referencePressure 参照気圧 [hPa]
  /// @param isSystemCalibrated BNO055システムのキャリブレーションが完了しているか
  /// @param isGyroscopeCalibrated BNO055角加速度計のキャリブレーションが完了しているか
  /// @param isAccelerometerCalibrated BNO055加速度計のキャリブレーションが完了しているか
  /// @param isMagnetometerCalibrated BNO055地磁気計のキャリブレーションが完了しているか
  void sendSensingStatus(float referencePressure, bool isSystemCalibrated, bool isGyroscopeCalibrated, bool isAccelerometerCalibrated, bool isMagnetometerCalibrated);

  /// @brief 電圧を送信する
  /// @param supply 供給電圧 [V]
  /// @param pool プール電圧 [V]
  /// @param battery バッテリー電圧 [V]
  void sendVoltage(float supply, bool pool, bool battery);


  /// @brief スカラー値を送信する
  /// @param label データの種類
  /// @param value 値
  void sendScalar(Label label, float value);

  /// @brief ベクトル値を送信する
  /// @param label データの種類
  /// @param axis 軸
  /// @param value 値
  void sendVector(Label label, Var::Axis axis, float value);

  /// @brief 3次元のベクトル値を送信する
  /// @param label データの種類
  /// @param xValue x軸の値
  /// @param yValue y軸の値
  /// @param zValue z軸の値
  void sendVector3D(Label label, float xValue, float yValue, float zValue);


  /// @brief システムステータスを受信する
  /// @param flightMode フライトモード
  /// @param cameraState カメラの状態
  /// @param sn3State 不知火3の状態
  /// @param doLogging ログ保存するか
  void receiveSystemStatus(Var::FlightMode* flightMode, Var::State* cameraState, Var::State* sn3State, bool* doLogging);

  /// @brief スカラー値を受信する
  /// @param value 値のポインタ
  void receiveScalar(float* value);

  /// @brief 3次元のベクトル値を受信する
  /// @param xValue x軸の値のポインタ
  /// @param yValue y軸の値のポインタ
  /// @param zValue z軸の値のポインタ
  void receiveVector3D(float* xValue, float* yValue, float* zValue);

  /// @brief 参照気圧セットを受信する
  /// @param referencePressure 参照気圧のポインタ
  void receiveSetReferencePressure(float* referencePressure);

  /// @brief 軌道情報の受信する
  /// @param isFalling true: 落下中, false: 落下中でない
  void receiveTrajectoryData(bool* isFalling);

  /// @brief バルブ制御モードを受信する
  /// @param isWaiting true: WAITING, false: LAUNCH
  void receiveValveMode(bool* isWaiting);


private:
  uint32_t _latestLabel;
  uint8_t _latestData[8];
};