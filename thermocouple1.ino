/**
* LENR logger thermocouple 1 - reactor core temp - related functions
*/
//settings
const int thermoDO1 = 3;
const int thermoCS1 = 4;
const int thermoCLK1 = 5;
const int thermocoupleMaxRead1 = 10; // number of readings to take before creating avg value
const long readThermocoupleInterval1 = 100;//read every 500 millisecs
/**
* LL_TC_USE_AVG
* set to one for TC's to use averages
*/
#define LL_TC_USE_AVG 1

//vars
int thermocoupleReadCount1 = 0; // number of times sensor has been read
float thermocoupleTotalReadingCelsius1 = 0.000; //total readings to avg
//float thermocoupleTotalReadingFahrenheit1 = 0.000; //total readings to avg
unsigned long readThermocoupleMillis1 = 0; // last milli secs since last avg reading
float thermocoupleAvgCelsius1 = -200.000; // thermocoupleTotalReadingCelsius div thermocoupleReadCount
// -200.000 means reading has not started yet so do not use
/**
* objects for lib classes
*/
Adafruit_MAX31855 thermocouple1(thermoCLK1, thermoCS1, thermoDO1);

/**
* Read reactor thermocouple Celsius value and
* create avg every thermocoupleReadCount times
*/
void readThermocouple1() 
{
  if (millis() - readThermocoupleMillis1 >= readThermocoupleInterval1) {
    readThermocoupleMillis1 = millis();
#if LL_TC_USE_AVG == 1
     thermocoupleReadCount1++;
   //thermocoupleAvgCelsius1 = thermocouple1.readCelsius();
   thermocoupleTotalReadingCelsius1 += thermocouple1.readCelsius();
   if (thermocoupleReadCount1==thermocoupleMaxRead1) {
      thermocoupleAvgCelsius1 = thermocoupleTotalReadingCelsius1/(float) thermocoupleMaxRead1;
      thermocoupleReadCount1 = 0;      
      thermocoupleTotalReadingCelsius1 = 0;
   }
#else
   thermocoupleAvgCelsius1 = thermocouple1.readCelsius();
#endif    
  }
}

float getThermocoupleAvgCelsius1() 
{
  return thermocoupleAvgCelsius1;
}
