# H-58 搭載計器

## 資料

- [工程計画書](./Documents/Materials/Schedule.md)
- [要件定義書](./Documents/Materials/RequirementsDocument.md)
- [お買い物リスト](./Documents/Materials/OKAIMONOList.md) (2023年度すぐに買うもの)

### 開発資料

- [データ回収](./Documents/Materials/Development/%E9%96%8B%E7%99%BA%E8%B3%87%E6%96%99_%E3%83%87%E3%83%BC%E3%82%BF%E5%9B%9E%E5%8F%8E.pdf)

## 標準規格

### 基板

- [標準モジュール](./Components/StandardModuleBoard/) ... STM32とCAN-BUSへの接続を実装した基板

### 接続

- [標準バスパワーライン](./Components/StandardBusPowerLine/) ... 12V電源とCAN-BUSからなる接続規格

## モジュール

- [計測モジュール](./Components/SensingModule/) ... センサ, データ算出
- 飛翔管理モジュール ... フライトモード管理, 分離制御
- [通信モジュール](./Components/CommunicationModule/) ... アップリンク, ダウンリンク
- [THR制御モジュール](./Components/THRControlModule/) ... 推進系計測, バルブ制御

## HOW TO

- [シンボリックリンク作り方](./Documents/Materials/HowTo/MakeSymbolicLink.md)
