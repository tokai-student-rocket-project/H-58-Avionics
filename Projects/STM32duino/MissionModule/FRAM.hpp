#pragma once


#include <Arduino.h>
#include <SPI.h>


/// @brief MB85RS2MTの抽象化クラス
class FRAM {
public:
  /// @brief アドレスの最大値
  static const uint32_t LENGTH = 262144;


  /// @brief コンストラクタ
  /// @param cs SPIのチップセレクト
  FRAM(uint32_t cs);


  /// @brief ライトイネーブルをONにする これをしないと書き込めない
  void setWriteEnable();

  /// @brief FRAMのステータスを返す
  /// @param buffer データ長1のバッファ
  void getStatus(uint8_t* buffer);

  /// @brief FRAMのIdを返す
  /// @param buffer データ長3のバッファ
  void getId(uint8_t* buffer);


  /// @brief データを読み出す
  /// @param address アドレス
  /// @return データ
  uint8_t read(uint32_t address);

  /// @brief データを書き込む
  /// @param address アドレス
  /// @param data データ
  void write(uint32_t address, uint8_t data);

  /// @brief 可変長のデータを書き込む
  /// @param address アドレス
  /// @param data データ
  /// @param size データ長
  void write(uint32_t address, const uint8_t* data, uint32_t size);


  /// @brief 全てに0を書き込み初期化する ブロッキング処理で時間がかかる
  void clear();

  /// @brief 全てのデータをシリアルに出力する ブロッキング処理で時間がかかる
  void dump();

private:
  typedef enum {
    WREN = 0b00000110,
    RDSR = 0b00000101,
    READ = 0b00000011,
    WRITE = 0b00000010,
    RDID = 0b10011111
  } ope_code_t;

  SPISettings _setting;
  uint32_t _cs;
};