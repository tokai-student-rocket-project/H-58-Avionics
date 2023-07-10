#pragma once


#include <Arduino.h>
#include <SPI.h>
#include <SD.h>


/// @brief SDカード用ライブラリ
class Sd {
public:
  /// @brief コンストラクタ
  /// @param cs SDのチップセレクト
  Sd(uint32_t cs);


  /// @brief 保存を開始する この時にファイルが生成される
  /// @return true: 開始成功, false: 開始失敗
  bool beginLogging();

  /// @brief 保存を終了する ファイルの後始末もする
  void endLogging();


  void write(const uint8_t* data, uint32_t size);

private:
  uint32_t _cs;
  bool _isRunning = false;

  File _logFile;
};