/**
* LENR logger
*
* Prints out results to CSV strings and Serial 
*/

/**
* get CSV string of headers for all enabled sensors
*/
String getCsvStringHeaders()
{
   String ret = String("\"Time\"");
   
   if (isSensorEnabled("TC1")) {     
     ret = ret + ",\"Thermocouple1\"";
   }
   if (isSensorEnabled("TC2")) {
     ret = ret +",\"Thermocouple2\"";
   }
   if (isSensorEnabled("Power")) {
     ret = ret +",\"Power\"";
   }
   if (isSensorEnabled("Pressure")) {
     ret = ret +",\"PSI\"";
   }
   
   return ret;
}


/**
* Get CSV string of all enabled sensor readings
*/
String getCsvString()
{
   String ret = String(millis());
    char s_[15];
    
   if (isSensorEnabled("TC1")) {
     dtostrf(getThermocoupleAvgCelsius1(),2,3,s_);
     String tmp = String(s_);
     ret = ret + ','+tmp;
     
   }
   if (isSensorEnabled("TC2")) {     
     dtostrf(getThermocoupleAvgCelsius2(),2,3,s_);
     String tmp = String(s_);
     ret = ret +','+tmp;
   }
   if (isSensorEnabled("Power")) {
    
     dtostrf(getPower(),2,3,s_);
     String tmp = String(s_);
     ret = ret +','+tmp;
   }
   if (isSensorEnabled("Pressure")) {
     String tmp = String(getPressurePsi());
     ret = ret +','+tmp;
   }
   
   return ret;
}

/**
* Print headers in to file if writing a line for the first time since opening
*/
boolean sdCardFirstLineLogged = false;

/**
* Save all enabled sensors values to SD card CSV file DATALOG
*/
void saveCsvData()
{
  if (!sdCardFirstLineLogged) {
    saveLineToDatalog(getCsvStringHeaders());
 }
 sdCardFirstLineLogged = true;
 saveLineToDatalog(getCsvString()); 
 
  if (debugToSerial) {
      Serial.println("csv data saved to datalog file");
    }
}

/**
* Print result to serial 0  for interfacing with a program on a PC over USB
*/
void printRawCsv() {    
  Serial.println(getCsvString());
}


