#pragma once


#include <Arduino.h>
#include <ACAN_STM32.h>

/// @brief CANコントローラ内蔵STM32用クラス
class CANSTM {
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
    SET_REFERENCE_PRESSURE,
    TRAJECTORY_DATA
  };

  /// @brief ベクトル用のxyzを列挙型で定義しておく
  // TODO 摘出
  enum class Axis : uint8_t {
    X,
    Y,
    Z
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
/// @param cameraState true: ON, false: OFF
/// @param sn3State true: ON, false: OFF
  // TODO 摘出した列挙型に変更
  void sendSystemStatus(uint8_t flightMode, bool cameraState, bool sn3State);

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


  /// @brief スカラー値を送信する
  /// @param label データの種類
  /// @param value 値
  void sendScalar(Label label, float value);

  /// @brief ベクトル値を送信する
  /// @param label データの種類
  /// @param axis 軸
  /// @param value 値
  void sendVector(Label label, Axis axis, float value);

  /// @brief 3次元のベクトル値を送信する
  /// @param label データの種類
  /// @param xValue x軸の値
  /// @param yValue y軸の値
  /// @param zValue z軸の値
  void sendVector3D(Label label, float xValue, float yValue, float zValue);


  /// @brief システムステータスを受信する
/// @param flightMode フライトモード
/// @param cameraState true: ON, false: OFF
/// @param sn3State true: ON, false: OFF
  // TODO 摘出した列挙型に変更
  void receiveSystemStatus(uint8_t* flightMode, bool* cameraState, bool* sn3State);

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

private:
  uint32_t _latestLabel;
  uint8_t _latestData[8];
};