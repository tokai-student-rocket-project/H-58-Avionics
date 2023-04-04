#include <Wire.h>


class BNO055 {
private:
public:
  const uint8_t SLAVE_ADDRESS = 0x28;

  const uint8_t REGISTER_PAGE = 0x07;

  const uint8_t REGISTER_OPR_MODE = 0x3D;
  const uint8_t OPR_MODE_CONFIGMODE = 0x00;
  const uint8_t OPR_MODE_NDOF = 0x0C;

  const uint8_t REGISTER_PWR_MODE = 0x3E;
  const uint8_t PWR_MODE_NORMALMODE = 0x00;

  const uint8_t REGISTER_SYS_TRIGGER = 0x3F;
  const uint8_t CLK_SEL = 0x80;

  const uint8_t RESISTER_ACC_DATA = 0x08;

  void initialize();

  void getAcceleration(double* x, double* y, double* z);
};