# H-58 搭載計器

## 資料

- [工程計画書](./Documents/Materials/Schedule.md)
- [要件定義書](./Documents/Materials/RequirementsDocument.md)
- [基本設計書](./Documents/Materials/BasicDesignDocument.md)
- [詳細設計書](./Documents/Materials/DetailedDesignDocument.md)

### 開発資料

- [データ回収](./Documents/Materials/Development/%E9%96%8B%E7%99%BA%E8%B3%87%E6%96%99_%E3%83%87%E3%83%BC%E3%82%BF%E5%9B%9E%E5%8F%8E.pdf)

## 標準規格

### 基板

- [標準モジュール](./Components/StandardModuleBoard/) ... STM32とCAN-BUSへの接続を実装した基板

### 接続

- [標準バスパワーライン](./Components/StandardBusPowerLine/) ... 12V電源とCAN-BUSからなる接続規格

#### CAN-BUS

SM ... SensingModule  
FM ... FlightModule  
MM ... MissionModule  
ACM ... AirDataCommunicationModule  
SCM ... SystemDataCommunicationModule  

| Id   | Label                  | length | From | To          | Rate | Format                                                                                                                                                                        |
| ---- | ---------------------- | ------ | ---- | ----------- | ---- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0x00 | Orientation            | 5B     | SM   | ACM         | 20Hz | [0] Axis (uint8_t), [1:5] Value (float)                                                                                                                                       |
| 0x01 | Acceleration           | 5B     | SM   | ACM         | 20Hz | [0] Axis (uint8_t), [1:5] Value (float)                                                                                                                                       |
| 0x02 | Altitude               | 4B     | SM   | FM, ACM     | 20Hz | [0:4] Value (float)                                                                                                                                                           |
| 0x03 | Outside Temperature    | 4B     | SM   | ACM         | 20Hz | [0:4] Value (float)                                                                                                                                                           |
| 0x04 | Voltage                | 6B     | FM   | SCM         | 10Hz | [0:2] Supply Voltage (int16_t), Pool Voltage [2:4] (int16_t), [4:6] Battery Voltage (int16_t) Voltage                                                                         |
| 0x05 | SystemStatus           | 8B     | FM   | SM, MM, SCM | 10Hz | [0] FlightMode (uint8_t), [1] CameraState (bool), [2] SeparationState (bool), [3] DoLogging (bool), [4:8] FlightTime (uint32_t)                                               |
| 0x06 | Event                  | 6B     |      | SCM         |      | [0] Publisher (uint8_t), [1] EventCode (uint8_t), [2:6] Timestamp (uint32_t)                                                                                                  |
| 0x07 | Error                  | 7B     |      | SCM         |      | [0] Publisher (uint8_t), [1] ErrorCode (uint8_t), [2] ErrorReason (uint8_t), [3:7] Timestamp (uint32_t)                                                                       |
| 0x08 | Set Reference Pressure | 4B     | SCM  | SM          |      | [0:4] ReferencePressure (float)                                                                                                                                               |
| 0x09 | Trajectory Data        | 1B     | SM   | FM          |      | [0] IsFalling (bool)                                                                                                                                                          |
| 0x10 | Sensing Status         | 8B     | SM   | SCM         |      | [0:4] ReferencePressure (float), [4] IsSystemCalibrated (bool), [5] IsGyroscopeCalibrated (bool), [6] isAccelerometerCalibrated (float), [7] isMagnetometerCalibrated (float) |

## モジュール

- [計測モジュール](./Components/Modules/SensingModule/) ... センサ, データ算出
- [フライトモジュール](./Components/Modules/FlightModule/) ... フライトモード管理, 分離制御
- [通信モジュール](./Components/Modules/CommunicationModule/) ... アップリンク, ダウンリンク
- [THR制御モジュール](./Components/THRControlModule/) ... 推進系計測, バルブ制御
- [ミッションモジュール](./Components/Modules/MissionModule/) ... 1kHz加速度計測

- [電源系](./Components/Modules/PowerModule/)

## HOW TO

- [シンボリックリンク作り方](./Documents/Materials/HowTo/MakeSymbolicLink.md)
