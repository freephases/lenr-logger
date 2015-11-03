/**
*  LENR logger - SD Card and config/settings file methods
*
* setting are kept in RUN and data is logged to DATALOG files on SD card
*/


// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
//
// Chip Select pin is tied to pin 8 on the SparkFun SD Card Shield
const int chipSelect = 53;  

void sdCardSetup()
{
 
  if (DEBUG_TO_SERIAL == 1) {
    Serial.print("Initializing SD card...");
  }
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(chipSelect, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card failed, or not present");
    // don't do anything more:
    while(true) {      
      connectionOkLight.toggle();
      delay(1000);
    }
  }
//  if (DEBUG_TO_SERIAL == 1) {
//    Serial.println("card initialized.");
//  }
  
  loadConfig();  
   
}

/**
* Appends string to log file 'DATALOG' on SD card
*/
void saveLineToDatalog(String data)
{
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) 
  {    
    dataFile.println(data);
    dataFile.close();    
  }  
  else
  {
    Serial.println("error opening datalog.txt");
    while(true) {      //don't do anything else now, just flash the led on front of device
      connectionOkLight.toggle();
      delay(500);
    }
  } 
}




