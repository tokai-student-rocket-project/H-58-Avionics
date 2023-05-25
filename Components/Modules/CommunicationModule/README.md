# 通信モジュール

高速ダウンリンクと低速ダウンリンク及びアップリンクを備え、LoRaで地上と通信する。

## 構成

### 高速ダウンリンク, アップリンク

![CommunicationModule.png](../../Projects/Drawio/CommunicationModule/CommunicationModule.drawio.png)

### 低速ダウンリンク

![CommunicationModuleGNSS.png](../../Projects/Drawio/CommunicationModule/CommunicationModuleGNSS.drawio.png)

## 基板形状

標準モジュール基板を継承

マイコンはSTMを使わず、Arduino MKR WAN 1310 を使う。

## 仕様

### 高速ダウンリンク

必要な情報を絞り、高頻度(100Hz程度)で送信する。

- 速度
- 姿勢角
- 高度

### 低速ダウンリンク

機体の情報やGNSSなど様々な情報を低頻度(1~10Hz程度)で送信する。

- GNSS
- 制御状況
- フライトモード
- 電圧
- コンフィグ
- イベント通知

### アップリンク

地上局からのコマンドを受信する。

- コンフィグ設定
- フライトモードON
- 手動分離制御
