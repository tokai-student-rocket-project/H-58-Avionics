# ソフトウェア

重要な関数は以下の7つ。

- `setup()`
- `loop()`
- `task02Hz()`
- `task2Hz()`
- `task20Hz()`
- `task50Hz()`
- `task100Hz()`

`setup()`は起動時に実行される関数。`loop()`は無限ループして実行される関数。`task○○Hz()`はその周波数で定期的に実行される関数。

基本的な動作は全て上記の関数に定義されていて、他の関数や変数は上記の関数から呼び出されているだけなので、一旦上記の関数を理解すればいい。

## `setup()`

起動時に実行される関数。
主に初期化処理が書かれている。以下は細分化した解説。

```cpp
  // デバッグ用シリアルポート
  // Serial.begin(115200);
  // while (!Serial);
  // delay(800);
```

デバッグ用のシリアルポートを初期化する処理。
`Serial.print()`などしたかったら、コメントアウトを外す。

```cpp
  // FRAMとSDの電源は常にON
  device::peripheral::recorderPower.on();
```

FRAMとSDへ電源供給を始める処理。
海打ちの際は水没によるショートを防ぐため、着水前に電源供給を止めることができる。H-58は陸打ちなので常にONでいい。

```cpp
  SPI.setMOSI(A6);
  SPI.setMISO(A5);
  SPI.setSCLK(A4);
  SPI.begin();
```

SPI通信を開始する処理。
STM32duinoは使用するピンを明示的に指定しないと動かない仕様。

```cpp
  Wire.setSDA(D4);
  Wire.setSCL(D5);
  Wire.begin();
  Wire.setClock(400000);
```

I2C通信を開始する処理。例によってピンは明示的に指定。クロックは標準的な400kHzにしている。

```cpp
  device::sensor::bno.begin();
  device::sensor::bme.begin();
  device::sensor::thermistor.initialize();
```

センサ類を初期化する処理。
細かい処理は自作のセンサライブラリを参照。

```cpp
  canbus::can.begin();
  canbus::can.sendEvent(CANSTM::Publisher::SENSING_MODULE, CANSTM::EventCode::SETUP);
```

CAN通信を開始する処理。
開始後にはセットアップイベントを発行している。

```cpp
  // CANで送り損ねないように若干ずらしたレートにする
  Tasks.add(internal::task02Hz)->startIntervalSec(5);
  Tasks.add(internal::task2Hz)->startFps(2);
  Tasks.add(internal::task20Hz)->startFps(29);
  Tasks.add(internal::task50Hz)->startFps(53);
  Tasks.add(internal::task100Hz)->startFps(100);
```

後述する定期実行タスクを登録する処理。
FPSが中途半端な値になっているのは、CANにデータを送信する際にタイミングが重なるのを防ぐため。

## `loop()`

無限ループで実行される関数。
以下は細分化した解説。

```cpp
  Tasks.update();
```

タスクを更新する処理。TaskManager.hライブラリで必要。

```cpp
  //CAN受信処理
  if (canbus::can.available()) {
    switch (canbus::can.getLatestMessageLabel()) {
    case CANSTM::Label::SYSTEM_STATUS:
      canbus::handleSystemStatus();
      device::indicator::canReceive.toggle();
      break;
    case CANSTM::Label::SET_REFERENCE_PRESSURE:
      canbus::handleSetReferencePressure();
      device::indicator::canReceive.toggle();
      break;
    }
  }
```

CANの受信処理。

CANを受信したときは、switch文で欲しいデータかを判定して受け取る。計測モジュールでは**SYSTEM_STATUS**と**SET_REFERENCE_PRESSURE**が欲しいデータ。

`canbus::handleSystemStatus()`と`canbus::handleSetReferencePressure()`はデータを受け取った後の具体的な処理がまとめられている。`loop()`の中に長々と処理を書きたくなかったので別にした。

受け取った後はLEDをトグルで切り替えて点滅させる。

## `task02Hz()`

0.2Hzで実行したい処理。

```cpp
  // そんなものはない
```

開発段階で0.2Hzで実行したい処理は消滅した。

## `task2Hz()`

2Hzで実行したい処理。

```cpp
  // SDの検知
  if (device::detection::cardDetection.is(Var::SwitchState::CLOSE)) {
    // SDを検知した時はLED常時点灯
    device::indicator::sdStatus.on();
  }
  else {
    // SDが検知できない時はLED点滅
    device::indicator::sdStatus.toggle();
  }
```

マイクロSDカードの抜き差し検知。

検知できればLEDを常時点灯、検知できなければLEDを点滅させるだけ。

## `task20Hz()`

20Hzで実行したい処理。

```cpp
  // 地磁気はセンサからのODRが20Hzなので20Hzで読み出す
  device::sensor::bno.getMagnetometer(&data::magnetometer_x_nT, &data::magnetometer_y_nT, &data::magnetometer_z_nT);
  // CAN送信が20Hzなので、外気温はそれに合わせて20Hzで読み出す
  device::sensor::thermistor.getTemperature(&data::outsideTemperature_degC);
```

地磁気と外気温の計測。センサから値を取り出すときは基本的に変数をポインタ渡ししている。

## `task50Hz()`

```cpp
  // 気圧と気温から高度を算出する
  // 内部的には落下検知の処理もやっている
  internal::trajectory.update(data::pressure_hPa, data::outsideTemperature_degC);

  data::altitude_m = internal::trajectory.getAltitude();
  data::climbRate_mps = internal::trajectory.getClimbRate();
```

高度算出と上昇率算出。

```cpp
  // CANにデータを流す
  // 安全のため、高度50m以上でないと落下判定しない
  canbus::can.sendTrajectoryData(internal::trajectory.isFalling() && data::altitude_m >= 50.0);
  device::indicator::canSend.toggle();
```

CANに軌道データを送信する

## `task100Hz()`

```cpp
  // BNO055からのデータは基本的に100Hzで読み出す
  device::sensor::bno.getAcceleration(&data::acceleration_x_mps2, &data::acceleration_y_mps2, &data::acceleration_z_mps2);
  device::sensor::bno.getGyroscope(&data::gyroscope_x_dps, &data::gyroscope_y_dps, &data::gyroscope_z_dps);
  device::sensor::bno.getOrientation(&data::orientation_x_deg, &data::orientation_y_deg, &data::orientation_z_deg);
  device::sensor::bno.getLinearAcceleration(&data::linear_acceleration_x_mps2, &data::linear_acceleration_y_mps2, &data::linear_acceleration_z_mps2);
  device::sensor::bno.getGravityVector(&data::gravity_x_mps2, &data::gravity_y_mps2, &data::gravity_z_mps2);
  device::sensor::bno.getQuaternion(&data::quaternion_w, &data::quaternion_x, &data::quaternion_y, &data::quaternion_z);
  // 高度も解析用にできるだけ早い100Hzで読み出したい
  device::sensor::bme.getPressure(&data::pressure_hPa);
```

センサからデータを読み出す

```cpp
  // doLoggingのフラグが立っている時はログを保存する
  // 内部的にはFRAMとSDに書き込んでいる
  if (device::peripheral::logger.isLogging()) {
    device::peripheral::logger.log(
      millis(), static_cast<uint8_t>(data::flightMode),
      data::outsideTemperature_degC, data::pressure_hPa, data::altitude_m, internal::trajectory.climbIndex(), internal::trajectory.isFalling(),
      data::acceleration_x_mps2, data::acceleration_y_mps2, data::acceleration_z_mps2,
      data::gyroscope_x_dps, data::gyroscope_y_dps, data::gyroscope_z_dps,
      data::magnetometer_x_nT, data::magnetometer_y_nT, data::magnetometer_z_nT,
      data::orientation_x_deg, data::orientation_y_deg, data::orientation_z_deg,
      data::linear_acceleration_x_mps2, data::linear_acceleration_y_mps2, data::linear_acceleration_z_mps2,
      data::gravity_x_mps2, data::gravity_y_mps2, data::gravity_z_mps2,
      data::quaternion_w, data::quaternion_x, data::quaternion_y, data::quaternion_z
    );
  }
```

自作のLoggerライブラリを使ってデータを保存する。