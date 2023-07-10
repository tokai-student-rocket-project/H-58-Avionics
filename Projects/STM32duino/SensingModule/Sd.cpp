#include "Sd.hpp"


/// @brief コンストラクタ
/// @param cs SDのチップセレクト
Sd::Sd(uint32_t cs) {
  _cs = cs;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
}


/// @brief 保存を開始する この時にファイルが生成される
/// @param fileName ログファイルの名前 拡張子は.txtか.csv
/// @return true: 開始成功, false: 開始失敗
bool Sd::beginLogging(String fileName) {
  _isRunning = SD.begin(_cs);
  if (_isRunning) {
    _logFile = SD.open(fileName, FILE_WRITE);
  }

  return _isRunning;
}


/// @brief 保存を終了する ファイルの後始末もする
void Sd::endLogging() {
  if (_logFile) {
    _logFile.close();
  }

  SD.end();
  _isRunning = false;
}
