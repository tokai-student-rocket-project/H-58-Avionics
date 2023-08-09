# バルブ制御モジュール

![バルブ制御モジュール](./docs/images/ValveControlModuleBoard.JPG)

## 概要

### ハードウェア
バルブを制御するために取り付けてあるサーボモータの制御を担当している子．

- **Arduino NANO Every**
  - Arduino NANO Family の一員．
  - Arduino 純正品なので使いやすいが，コンパイルが少し遅い．
  
- **φ110基板**
  - φ150機体にちょうど収まり，配線も行いやすいようになっている．
  - 画像下にある切り欠きは配線をまとめるために工夫を行った．

- **RS485トランシーバーを搭載**
  - サーボモータと通信を行う．

- **ブザーを搭載**
  - 機体内から音で現在の状態を把握することが可能．

- **CAN-BUSを搭載**
  - CAN通信を各モジュール間で行うことで通信量/速度の向上．

### ソフトウェア
プログラム初心者なのでほとんどが拾ってきたもの．
特に，サーボモータを販売している近藤科学さんの[B3MSoftwareManual](../../../Documents/Datasheets/B3M/B3M_SoftwareManual1.2.0.3.pdf)と[B3Mサーボモータを動かそう(Arduino制御編) | 近藤科学](https://kondo-robot.com/faq/b3m_tutorial5r)にとてもお世話になった．

**使用したライブラリ**
- [ICS Library for Arduino ver.2 | 近藤科学](https://kondo-robot.com/faq/ics-library-a2)
  - 別のサーボモータに使用している関数の一部を流用している．
- [TaskManager | Github](https://github.com/hideakitai/TaskManager/tree/master)
  - タスク（行いたい処理）を管理するのにとても便利．例えばある動作を特定の周波数で作動させたいときとか．
- [mcp_2515 | Github](https://github.com/Seeed-Studio/Seeed_Arduino_CAN)
  - CAN通信を担当している[MCP2515](https://akizukidenshi.com/download/ds/microchip/mcp2515_j.pdf)のライブラリ．とても便利．

## パージした機能

- 熱電対による温度計測．
  - 発注した部品が届かなかった．残念．
  - 左側にある大きな四角い形をしたシルクは熱電対の名残．搭載しようとした理由として，センサモジュールではなく，センサを購入してセンサ部を作成してみたかったという理由もある．これにより，手はんだで小さなパッケージのセンサを運用できるのか試してみるという狙いがあった．

## 設計図

### 回路図&ボード図

👉 [ValveControlModule (THRControlModule)](../../../Projects/KiCad/THRControlModule/THRControlModule.kicad_pro)

Kicad ProjectsファイルになっているのでKicadで開けばボード図などもすべて紐づいている．

### 部品表

[THRControlModule.csv](./docs/THRControlModule.csv) を参照のこと．なお，名称がValveControlModuleではなく，THRControlModuleになっている点については触れず，ValveControlModuleに頭の中で置き換えて考えてほしい．(ごめん．)

熱電対部を実装していない関係で以下の表に示す部品は実装しなくてよい．

| Reference | 名称
| -- | -- |
| J6 | K熱電対コネクタ |
| R18 | 100Ω 抵抗 |
| R21 | 100Ω 抵抗 |
| C13, C15 | 0.01μF キャパシタ |
| C11, C12, C14 | 0.1μF キャパシタ|
| U3 | MAX31856 熱電対デジタルコンバータ |
| R17, R19, R20| 10kΩ 抵抗 |
| D6, D7, D8 | 1N4148 ダイオード|

## 製作過程を覗きたい方へ

[Issues | Github](https://github.com/tokai-student-rocket-project/H-58-Avionics/issues)の Close Issues を覗いてほしい．つまずいた箇所や気を付けるべきポイントが多少記載されている．

## つくったひと
 1CES2116 堤 大樹 
 
 ✉ 1ces2116@mail.u-tokai.ac.jp / hiroki1815@outlook.jp / hiroki1815tsutsumi@gmail.com