#include "Sd.hpp"


/// @brief コンストラクタ
/// @param cs SDのチップセレクト
Sd::Sd(uint32_t cs) {
  _cs = cs;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
}


/// @brief 保存を開始する この時にファイルが生成される
/// @return true: 開始成功, false: 開始失敗
bool Sd::beginLogging() {
  _isRunning = SD.begin(_cs);
  if (_isRunning) {
    uint8_t fileNumber = 0;
    while (fileNumber < 256) {
      String fileName = String(fileNumber) + ".txt";
      if (SD.exists(fileName)) {
        fileNumber++;
        continue;
      }

      _logFile = SD.open(fileName, FILE_WRITE);
      break;
    }
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


void Sd::write(const uint8_t* data, uint32_t size) {
  if (_logFile) {
    _logFile.write(data, size);
  }
}
