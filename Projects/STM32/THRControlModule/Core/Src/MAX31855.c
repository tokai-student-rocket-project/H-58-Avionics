#include"MAX31855.h"
SPI_HandleTypeDef hspi1;

// -- Variables --

uint8_t Error=0;
uint32_t SignBit=0;		//Sign Bit
uint8_t DATARX[4];		//Raw Data from MAX31655

float MAX31855_Read_Temp(void)
{
	int Temp = 0;
	HAL_GPIO_WritePin(SSPORT, SSPIN, GPIO_PIN_RESET);
	HAL_SPI_Receive(&hspi1, DATARX, 4, 1000);
	HAL_GPIO_WritePin(SSPORT, SSPIN, GPIO_PIN_SET);
	Error=DATARX[3]&0x07;
	SignBit=(DATARX[0]&(0x80))>>7;

	if(DATARX[3] & 0x07)

	return(-1*(DATARX[3] & 0x07));

	else if(SignBit==1){
		Temp = (DATARX[0] << 6) | (DATARX[1] >> 2);
		Temp&=0b01111111111111; //0111|1111|1111|11
		Temp^=0b01111111111111; //0111|1111|1111|11
		return((double)-Temp/4);
	}

	else
	{
		Temp = (DATARX[0] << 6 | (DATARX[1] >> 2));
		return((double)Temp /4);
	}
}
