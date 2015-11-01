/**
* LENR logger thermocouple 2 - room temp - related functions - should be same or very simular spec thermocouple to core thermocouple1
* if we need more thermocouples will be best to use object in an array 
*/
//settings
const int thermoDO2 = 33;//reanme as names do nt match my card DO is POO i think??? therefre i;m a rock
const int thermoCS2 = 31;
const int thermoCLK2 = 32;
const int thermocoupleMaxRead2 = 10; // number of readings to take before creating avg value
const long readThermocoupleInterval2 = 700;//read every 500 millisecs


//vars
int thermocoupleReadCount2 = 0; // number of times sensor has been read
float thermocoupleTotalReadingCelsius2 = 0.000; //total readings to avg
//float thermocoupleTotalReadingFahrenheit2 = 0.000; //total readings to avg
unsigned long readThermocoupleMillis2 = 0; // last milli secs since last avg reading
float thermocoupleAvgCelsius2 = -200.000; // thermocoupleTotalReadingCelsius div thermocoupleReadCount
// -200.000 means reading has not started yet so do not use
/**
* objects for lib classes
*/
MAX6675 thermocouple2(thermoCLK2, thermoCS2, thermoDO2);

/**
* Read room temp thermocouple Celsius value and
* create avg every thermocoupleReadCount times
*/
void readThermocouple2() {
  if (millis() - readThermocoupleMillis2 >= readThermocoupleInterval2) {
    readThermocoupleMillis2 = millis();
    thermocoupleReadCount2++;
    thermocoupleTotalReadingCelsius2 += thermocouple2.readCelsius();
    //thermocoupleTotalReadingFahrenheit2 +=thermocouple2.readFahrenheit();//leave for now

    if (thermocoupleReadCount2 == thermocoupleMaxRead2) {
      //set thermocoupleAvgCelsius and reset totals and counters
      thermocoupleAvgCelsius2 = thermocoupleTotalReadingCelsius2 / thermocoupleMaxRead2;
      thermocoupleTotalReadingCelsius2 = 0;
      //thermocoupleTotalReadingFahrenheit2 = 0;
      thermocoupleReadCount2 = 0;//reset to 0;
      if (DEBUG_TO_SERIAL == 1) {
        Serial.print("thermocouple2: ");
        Serial.println(thermocoupleAvgCelsius2);
      }
    }
  }
}

float getThermocoupleAvgCelsius2() {
  return thermocoupleAvgCelsius2;
}
