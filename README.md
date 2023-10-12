# H-58 搭載計器

![2023年度 - 夏季](https://img.shields.io/badge/2023年度-夏季-blue)
![射点 - 能代](https://img.shields.io/badge/射点-秋田県/能代市-orange)
![形式 - 陸打ち](https://img.shields.io/badge/形式-陸打ち-green)

TSRP-H-58に搭載する計器類のプログラム及び設計書を保存するレポジトリです。

![Hero](./Documents/Images/Hero.png)

## はじめに

学生ロケットの電装系を盛り上げたい思いから、全てのソースコードを含む設計を公開しています。ガバガバ開発ですので、分かりにくい点（資料がまとまっていないなど）が多々あります。ご了承ください。

ご不明な点は以下のSNSからお気軽にお問合せください。

- X (旧Twitter): [@Tokai_SRP](https://twitter.com/Tokai_SRP)

### 計器の概要

H-58搭載計器はCANBUS接続された複数のモジュールから構成されます。

### レポジトリ構成

#### ./Components/Modules/

- [SystemDataCommunicationModule](./Components/Modules/SystemDataCommunicationModule/README.md) ![通信系](https://img.shields.io/badge/通信系-blue) ![センサ系](https://img.shields.io/badge/センサ系-green)
- [AirDataCommunicationModule](./Components/Modules/AirDataCommunicationModule/README.md) ![通信系](https://img.shields.io/badge/通信系-blue)
- [FlightModule](./Components/Modules/FlightModule/README.md) ![分離制御系](https://img.shields.io/badge/分離制御系-yellow) ![計装系](https://img.shields.io/badge/計装系-purple)
- [SensingModule](./Components/Modules/SensingModule/README.md) ![センサ系](https://img.shields.io/badge/センサ系-green)
- [ValveControlModule](./Components/Modules/ValveContolModule/README.md) ![バルブ制御系](https://img.shields.io/badge/バルブ制御系-orange)
- [MissionModule](./Components/Modules/MissionModule/README.md) ![ミッション系](https://img.shields.io/badge/ミッション系-green)
- [PowerModule](./Components/Modules/PowerModule/README.md) ![電源系](https://img.shields.io/badge/電源系-red)

#### ./Documents/

- 開発資料
  - [工程計画書](./Documents/Materials/Schedule.md)
  - [要件定義書](./Documents/Materials/RequirementsDocument.md)
  - [基本設計書](./Documents/Materials/BasicDesignDocument.md)
  - [詳細設計書](./Documents/Materials/DetailedDesignDocument.md)
- 安全審査
  - [一次審査(Z-storage)](https://docs.google.com/document/d/1F-2k9H3952sz7PHYDDPXtUgwHEACfZ0U/edit?usp=sharing&ouid=102715455383884127068&rtpof=true&sd=true)
  - [二次審査(Z-storage)](https://drive.google.com/file/d/1VNDg2A9r9_NxHciDonn3Oh5dWTKx7AV1/view?usp=sharing)
- マニュアル
  - [部品一覧](./Documents/Materials/Manual/PartsList.md)
  - 手順書
    - [組み立て手順書](./Documents/Materials/Manual/Procedure/AssemblyProcedure.md)
- [詳細報告書](./Documents/Reports/DetailedRaport.pdf) (一番情報量多い)
