#include	"mmyMAX31855.h"

MAX31855::MAX31855(unsigned char SO, unsigned	char CS, unsigned char SCK)
{
	so = SO;
	cs = CS;
	sck = SCK;
	
	pinMode(so, INPUT);

	pinMode(cs, OUTPUT);

	pinMode(sck, OUTPUT);
	
	digitalWrite(cs, HIGH);
	digitalWrite(sck, LOW);
}

double	MAX31855::readThermocouple(unit_t	unit)
{
	unsigned long data;
	double temperature;
	
	temperature = 0;
	
	data = readData();
	
	if (data & 0x00010000)
	{
		switch (data & 0x00000007)
		{
			case 0x01:
				temperature = FAULT_OPEN;
				break;
			
			case 0x02:
				temperature = FAULT_SHORT_GND;
				break;
			
			case 0x04:
				temperature = FAULT_SHORT_VCC;
				break;
		}
	}
	else
	{
		data = data >> 18;
		temperature = (data & 0x00001FFF);

		if (data & 0x00002000)
		{
			data = ~data; 
			temperature = data & 0x00001FFF;
			temperature += 1;
			temperature *= -1; 
		}
		
		temperature *= 0.25;
		
		if (unit == FAHRENHEIT)
		{
			temperature = (temperature * 9.0/5.0)+ 32; 
		}
	}
	return (temperature);
}

double	MAX31855::readInternal(unit_t	unit)
{
	double	temperature;
	unsigned long data;
	
	data = readData();
	
	data = data >> 4;
	temperature = (data & 0x000007FF);
	
	if (data & 0x00000800)
	{
		data = ~data; 
		temperature = data & 0x000007FF;
		temperature += 1;	
		temperature *= -1; 
	}
	
	temperature *= 0.0625;
	
	if (unit == FAHRENHEIT)
	{
		temperature = (temperature * 9.0/5.0)+ 32; 	
	}
	
	return (temperature);
}

unsigned long MAX31855::readData()
{
	int bitCount;
	unsigned long data;
	
	data = 0;

	digitalWrite(cs, LOW);
	
	for (bitCount = 31; bitCount >= 0; bitCount--)
	{
		digitalWrite(sck, HIGH);
		
		if (digitalRead(so))
		{
			data |= ((unsigned long)1 << bitCount);
		}	
		
		digitalWrite(sck, LOW);
	}
	
	digitalWrite(cs, HIGH);
	
	return(data);
}