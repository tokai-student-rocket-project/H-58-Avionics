# 部品一覧

運用時の部品一覧です。基板実装に必要な部品などは各モジュールのREADMEを参照してください。

## 構造

### 計器ベースプレート

![計器ベース](./images/parts/structure/AvionocsBase.JPG)

標準カプラに計器ベープレートが締結され、スペーサーとアンビリカルケーブルコネクタパネルが取り付けられた状態。

### 締結部品

![締結部品](./images/parts/structure/Bolts.JPG)

左から

- M4 ナット (4個)
- M4x10mm スペーサー (28個)
- M4x20mm スペーサー (4個)
- M4 ワッシャー (24個)
- アンテナ固定用タイラップ (6個)

## 基板

裏面に基板の名称とソースコードへのQRコードが記載されているので参考にしてください。

### 電池ボックス基板

![電池ボックス基板](./images/parts/boards/BatteryBoard.JPG)

### 変圧基板

![変圧基板](./images/parts/boards/TransformationBoard.JPG)

### バルブ制御モジュール基板

![バルブ制御モジュール基板](./images/parts/boards/ValveControlModuleBoard.JPG)

### 計測モジュール基板

![計測モジュール基板](./images/parts/boards/SensingModuleBoard.JPG)

### フライトモジュール基板

![フライトモジュール基板](./images/parts/boards/FlightModuleBoard.JPG)

### ミッションモジュール基板

![ミッションモジュール基板](./images/parts/boards/MissionModuleBoard.JPG)

### エアデータ通信モジュール基板

![エアデータ通信モジュール基板](./images/parts/boards/AirDataCommunicationModule.JPG)

### システムデータ通信モジュール基板

![システムデータ通信モジュール基板](./images/parts/boards/SystemDataCommunicationModuleBoard.JPG)

### カメラモジュール基板

![カメラモジュール基板](./images/parts/boards/CameraModuleBoards.JPG)

## 配線

![VREF](./images/parts/wire/VREF.JPG)

![UmbilicalConnector,CAMERA,SN3](./images/parts/wire/UmbilicalConnector,CAMERA,SN3.JPG)

### VREF

電源基板の各点電圧をフライトモジュールから監視するための配線。

### SN3

不知火への分離信号用配線。

### CAMERA

カメラモジュールへの録画信号用配線。

### アンビリカル中継ケーブル

アンビリカルケーブルコネクタパネルと外部のアンビリカルケーブルを中継する配線。

![PowerKey](./images/parts/wire/PowerKey.JPG)

### 電源キー

変圧モジュールの内部電源の鍵として使う配線。

![CakeMamoruKun](./images/parts/wire/CakeMamoruKun.JPG)

### ケーキ守るくん

変圧モジュール内のコンデンサによる再起電圧防止のため、無負荷時はケーキ守るくんを取り付けます。
