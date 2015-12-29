/**
* LENR logger thermocouple 1 - reactor core temp - related functions
*/
//settings
const int thermoDO1 = 3;
const int thermoCS1 = 4;
const int thermoCLK1 = 5;
//const int thermocoupleMaxRead1 = 10; // number of readings to take before creating avg value
const long readThermocoupleInterval1 = 1000;//read every 500 millisecs


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
void readThermocouple1() {
  if (millis() - readThermocoupleMillis1 >= readThermocoupleInterval1) {
    readThermocoupleMillis1 = millis();
   
   thermocoupleAvgCelsius1 = thermocouple1.readCelsius();
   
      if (debugToSerial) {
        Serial.print("thermocouple1: ");
        Serial.println(thermocoupleAvgCelsius1);
      }
    
  }
}

float getThermocoupleAvgCelsius1() {
  return thermocoupleAvgCelsius1;
}
