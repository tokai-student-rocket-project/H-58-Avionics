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

| Id   | Label           | From | To          | Rate | Format                                               |
| ---- | --------------- | ---- | ----------- | ---- | ---------------------------------------------------- |
| 0x00 | Orientation     | SM   | ACM         | 20Hz | [0]Axis,[1:5]Value                                   |
| 0x01 | Acceleration    | SM   | ACM         | 20Hz | [0]Axis, [1:5]Value                                  |
| 0x02 | Altitude        | SM   | FM, ACM     | 20Hz | [0:4] Value                                          |
| 0x03 | Temperature     | SM   | ACM         | 20Hz | [0:4] Value                                          |
| 0x04 | Supply Voltage  | FM   | SCM         | 10Hz | [0:4] Value                                          |
| 0x05 | Battery Voltage | FM   | SCM         | 10Hz | [0:4] Value                                          |
| 0x06 | Pool Voltage    | FM   | SCM         | 10Hz | [0:4] Value                                          |
| 0x07 | SystemStatus    | FM   | SM, MM, SCM | 10Hz | [0] FlightMode, [1] CameraState, [2] SeparationState |

## モジュール

- [計測モジュール](./Components/Modules/SensingModule/) ... センサ, データ算出
- [フライトモジュール](./Components/Modules/FlightModule/) ... フライトモード管理, 分離制御
- [通信モジュール](./Components/Modules/CommunicationModule/) ... アップリンク, ダウンリンク
- [THR制御モジュール](./Components/THRControlModule/) ... 推進系計測, バルブ制御
- [ミッションモジュール](./Components/Modules/MissionModule/) ... 1kHz加速度計測

- [電源系](./Components/Modules/PowerModule/)

## HOW TO

- [シンボリックリンク作り方](./Documents/Materials/HowTo/MakeSymbolicLink.md)
