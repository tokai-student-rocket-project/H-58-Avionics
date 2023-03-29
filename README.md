# H-58 搭載計器

## 開発資料

- [工程計画書](./Documents/Materials/Schedule.md)
- [要件定義書](./Documents/Materials/RequirementsDocument.md)
- [お買い物リスト](./Documents/Materials/OKAIMONOList.md) (2023年度すぐに買うもの)

## 標準規格

### 基板

- [標準モジュール](./Components/StandardModuleBoard/) ... STM32とCAN-BUSへの接続を実装した基板

### 接続

- [標準バスパワーライン](./Components/StandardBusPowerLine/) ... 12V電源とCAN-BUSからなる接続規格

## モジュール

- [計測モジュール](./Components/SensingModule/) ... センサ, データ算出
- 飛翔管理モジュール ... フライトモード管理, 分離制御
- [通信モジュール](./Components/CommunicationModule/) ... アップリンク, ダウンリンク
- THR制御モジュール ... 推進系計測, バルブ制御

## HOW TO

- [シンボリックリンク作り方](./Documents/Materials/HowTo/MakeSymbolicLink.md)
