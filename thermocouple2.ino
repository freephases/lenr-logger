/**
* LENR logger thermocouple 2 - room temp - related functions 
* gets temp via serial from mini pro 
*/
//settings
//const int thermoDO2 = 41;
//const int thermoCS2 = 40;
//const int thermoCLK2 = 39;
//const int thermocoupleMaxRead2 = 10; // number of readings to take before creating avg value
//const long readThermocoupleInterval2 = 1000;//read every 500 millisecs


//vars
//int thermocoupleReadCount2 = 0; // number of times sensor has been read
//float thermocoupleTotalReadingCelsius2 = 0.000; //total readings to avg
//float thermocoupleTotalReadingFahrenheit2 = 0.000; //total readings to avg
unsigned long readThermocoupleMillis2 = 0; // last milli secs since last avg reading
float thermocoupleAvgCelsius2 = -200.000; // thermocoupleTotalReadingCelsius div thermocoupleReadCount
float thermocoupleInternalCelsius2 = 0.00;
// -200.000 means reading has not started yet so do not use
/**
* objects for lib classes
*/
//Adafruit_MAX31855 thermocouple2(thermoCLK2, thermoCS2, thermoDO2);
SoftwareSerial thermocouple2Serial(10,33);

short tc2Pos = 0; // position in read buffer
char tc2Buffer[MAX_STRING_DATA_LENGTH + 1];
char tc2InByte = 0;


/**
* Set emontx struct
*/
void setThermocouple2Info() {
  char buf[15];
  float f;

  getValue(tc2Buffer, '|', 1).toCharArray(buf, 15);
  f = atof(buf);
  thermocoupleInternalCelsius2 = f;

  getValue(tc2Buffer, '|', 2).toCharArray(buf, 15);
  f = atof(buf);
  thermocoupleAvgCelsius2 = f;
  
  if (debugToSerial) {
        Serial.print("thermocouple2: ");
        Serial.println(thermocoupleAvgCelsius2);
      }

}

/**
* Execute response
*/
void processThermocouple2Response()
{
  char recordType = tc2Buffer[0];

  switch (recordType) {
    case 'R' : //got a response record/data from the our power slave
      setThermocouple2Info();
      break;

  }
}

void processThermocouple2Serial()
{
 // thermocouple2Serial.listen();
  while (thermocouple2Serial.available() > 0)
  {
    tc2InByte = thermocouple2Serial.read();

    // add to our read buffer
    tc2Buffer[tc2Pos] = tc2InByte;
    tc2Pos++;

    if (tc2InByte == '\n' || tc2Pos == MAX_STRING_DATA_LENGTH) //end of max field length
    {
      tc2Buffer[tc2Pos - 1] = 0; // delimited
      processThermocouple2Response();
      tc2Buffer[0] = '\0';
      tc2Pos = 0;
    }
  }
}


/**
* Read room temp thermocouple Celsius value and
* create avg every thermocoupleReadCount times
*/
void readThermocouple2() {
  processThermocouple2Serial();
}

float getThermocoupleAvgCelsius2() {
  return thermocoupleAvgCelsius2;
}

void thermocouple2Setup() {
 thermocouple2Serial.begin(9600);
}
