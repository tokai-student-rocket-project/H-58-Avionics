# LogSender.ino

ロングラン試験で必要な以下のログをLoRaで送信するためのコード。

- チューブ中雰囲気温度
- 供給電圧
- バッテリー電圧
- プール電圧

Arduino MKR WAN 1310 の SystemDataCommunicationModuleに書き込んで使う。

受信側の Arduino MKR WAN 1310 には LogReceiver.inoを書き込む。シリアルモニターにcsvで出力される。

GNSSなどSystemDataCommunicationModule本来の機能も動くので、消費電力的にはほとんど変わらないはず。
