/**
* Prints out results to serial as CSV, can be changed to match any format 
* to interface with anything via usb ports on the mega
*/
#define csvDelimiter ','

void printRawCsv() {    
  Serial.print(millis());  
  Serial.print(csvDelimiter);
  Serial.print(getThermocoupleAvgCelsius1());
  Serial.print(csvDelimiter);
  Serial.print(getThermocoupleAvgCelsius2());
  Serial.print(csvDelimiter);
  Serial.print(getPower());
  Serial.print(csvDelimiter);
  Serial.print(getPressurePsi());
}
