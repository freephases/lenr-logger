/**
* LENR logger thermocouple 1 - reactor core temp - related functions
*/
//settings
const int thermoDO1 = 29;//reanme as names do nt match my card DO is POO i think??? therefre i;m a rock
const int thermoCS1 = 27;
const int thermoCLK1 = 28;
const int thermocoupleMaxRead1 = 10; // number of readings to take before creating avg value
const long readThermocoupleInterval1 = 500;//read every 500 millisecs


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
MAX6675 thermocouple1(thermoCLK1, thermoCS1, thermoDO1);

/**
* Read reactor thermocouple Celsius value and
* create avg every thermocoupleReadCount times
*/
void readThermocouple1() {
  if (millis() - readThermocoupleMillis1 >= readThermocoupleInterval1) {
    readThermocoupleMillis1 = millis();
    thermocoupleReadCount1++;
    thermocoupleTotalReadingCelsius1 += thermocouple1.readCelsius();
    //thermocoupleTotalReadingFahrenheit1 +=thermocouple1.readFahrenheit();//leave for now

    if (thermocoupleReadCount1 == thermocoupleMaxRead1) {
      //set thermocoupleAvgCelsius and reset totals and counters
      thermocoupleAvgCelsius1 = thermocoupleTotalReadingCelsius1 / thermocoupleMaxRead1;
      thermocoupleTotalReadingCelsius1 = 0;
      //thermocoupleTotalReadingFahrenheit1 = 0;
      thermocoupleReadCount1 = 0;//reset to 0;
      if (DEBUG_TO_SERIAL == 1) {
        Serial.print("thermocouple1: ");
        Serial.println(thermocoupleAvgCelsius1);
      }
    }
  }
}

float getThermocoupleAvgCelsius1() {
  return thermocoupleAvgCelsius1;
}
