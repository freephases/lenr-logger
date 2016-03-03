/**
* LENR logger thermocouple 2 - reactor core temp - related functions
*/
//settings
const int thermoDO2 = 3;//same as tc1
const int thermoCS2 = 2;
const int thermoCLK2 = 5;//same as tc1
const int thermocoupleMaxRead2 = 10; // number of readings to take before creating avg value
const long readThermocoupleInterval2 = 100;//read every 500 millisecs

//runtime vars
int thermocoupleReadCount2 = 0; // number of times sensor has been read
float thermocoupleTotalReadingCelsius2 = 0.000; //total readings to avg
//float thermocoupleTotalReadingFahrenheit2 = 0.000; //total readings to avg
unsigned long readThermocoupleMillis2 = 0; // last milli secs since last avg reading
float thermocoupleAvgCelsius2 = -200.000; // thermocoupleTotalReadingCelsius div thermocoupleReadCount
// -200.000 means reading has not started yet so do not use
/**
* objects for lib classes
*/
Adafruit_MAX31855 thermocouple2(thermoCLK2, thermoCS2, thermoDO2);

/**
* Read reactor thermocouple Celsius value and
* create avg every thermocoupleReadCount times
*/
void readThermocouple2() 
{
  if (millis() - readThermocoupleMillis2 >= readThermocoupleInterval2) {
    readThermocoupleMillis2 = millis();
#if LL_TC_USE_AVG == 1
     thermocoupleReadCount2++;
   //thermocoupleAvgCelsius2 = thermocouple2.readCelsius();
   thermocoupleTotalReadingCelsius2 += thermocouple2.readCelsius();
   if (thermocoupleReadCount2==thermocoupleMaxRead2) {
      thermocoupleAvgCelsius2 = thermocoupleTotalReadingCelsius2/(float) thermocoupleMaxRead2;
      thermocoupleReadCount2 = 0;      
      thermocoupleTotalReadingCelsius2 = 0;
   }
#else
   thermocoupleAvgCelsius2 = thermocouple2.readCelsius();
#endif    
  }
}

float getThermocoupleAvgCelsius2() 
{
  return thermocoupleAvgCelsius2;
}
