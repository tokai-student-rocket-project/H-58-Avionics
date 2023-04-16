/*動作確認済み*/

#include  <mmyMAX31855.h>

const  unsigned  char thermocoupleSO = 12;
const  unsigned  char thermocoupleCS = 10;
const  unsigned  char thermocoupleCLK = 13;

MAX31855  MAX31855(thermocoupleSO, thermocoupleCS, thermocoupleCLK);

void  setup()
{
  Serial.begin(115200);
}

void  loop()
{
  double  temperature;
  

  temperature = MAX31855.readThermocouple(CELSIUS);
  Serial.print("Thermocouple temperature = ");
  Serial.print(temperature);
  Serial.println(" C");
  Serial.println("");
  

  temperature = MAX31855.readInternal(CELSIUS);
  Serial.print("Internal temperature = ");
  Serial.print(temperature);
  Serial.println(" C");
  
  delay(10);
}