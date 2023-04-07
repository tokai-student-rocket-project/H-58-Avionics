/*
 * MAX31855.h
 *
 *  Created on: 2023/04/06
 *      Author: Hiroki
 */

#ifndef INC_MAX31855_H_
#define INC_MAX31855_H_
#include "main.h"

extern uint8_t Error;
#define SSPORT GPIOA
#define SSPIN GPIO_PIN_4

float MAX31855_Read_Temp(void);
#endif /* INC_MAX31855_H_ */
