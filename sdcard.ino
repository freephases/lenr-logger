/**
*  LENR logger - SD Card and config/settings file methods
*
* setting are kept in RUN and data is logged to DATALOG files on SD card
*/


/*
;Example Settings file contents:
;lenr logger settings for run time
SSID=MyWifiNetWork
password=MyPassword
;sensors_enabled piped delimited list,  values are TC1-TC2, Power, Pressure, Light, IR, Geiger
sensors_enabled=TC1|TC2|Power|Pressure|!
plotly-username=username
plotly-password=password
;tokens piped delited list for each sensors_enabled in the same order as speicifed in sensors_enabled
plotly-tokens=dddhfjfj4e|dddhfjfj43|dddhfjf34e|dddhfjfj42
plotly-token-count=4
plotly-overwrite=yes
plotly-max-points=300
plotly-filename=test2
disable_sd_logging=no
send_interval_sec=15
*/

#define MAX_SETTINGS 14
byte fileChar;
int fileBufPos = 0;
int fileLine = 0;
char fileBuf[70];
char loggerSettings[MAX_SETTINGS][70]= {"", "", "", "", "", "", "", "", "", "", "", "", "",""};
int settingsCount = 0;
//settings in file 
boolean ignoreUntilNewLine = false;
boolean sdCardFirstLineLogged = false;

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
* Populate global settings from our files config settings
*/
void loadGlobalSettings() {
   //load up default config settings  
   
 
  getConfigSetting("plotly-username").toCharArray(plotlyUserName, 30);
  getConfigSetting("plotly-password").toCharArray(plotlyPassword, 20);
  getConfigSetting("plotly-filename").toCharArray(plotlyFilename, 40);
  getConfigSetting("plotly-tokens").toCharArray(plotlyTokens, 70);
  
  logToSDCard = !getConfigSettingAsBool("disable_sd_logging");
  
  unsigned long sI = getConfigSettingAsInt("send_interval_sec");
  if (sI<1) sI = defaultSendDataIntervalSecs;
  sendDataInterval = sI*1000;
}

/**
* Apands string to log file on SD card
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
    while(true) {      //don't do anything else now
      connectionOkLight.toggle();
      delay(500);
    }
  } 
}


/**
* Load config file from SD card
*/
void loadConfig()
{
  // re-open the file for reading:
  File myFile = SD.open("run.txt");//file must not have txt ext on device - not sure why
  if (myFile) {
    if (DEBUG_TO_SERIAL == 1) {
      Serial.println("opened file to read run.txt:");
    }
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
     fileChar = myFile.read();
     if (fileChar == '\r') continue;
     if (fileBufPos==0 && fileChar == ';') { //ignore line
       ignoreUntilNewLine = true;
       continue;
     }
     if (ignoreUntilNewLine) {
       if (fileChar == '\n') {
         ignoreUntilNewLine = false;
       }
       continue;
     }
     else if (fileChar != '\n') {
      loggerSettings[fileLine][fileBufPos] = fileChar;
      fileBufPos++;
     } else {     
       if (fileBufPos==0) { //ignore empty lines
         continue;
       }  
       loggerSettings[fileLine][fileBufPos] = '\0';
       //increment and reset
       fileLine++;
       fileBufPos = 0;
       if (fileLine==MAX_SETTINGS) {
         break;
       }
     }     
    }
    //finish off last line ending if there was nor line ending in config file
    if (fileBufPos!=0 && fileLine<10) {
      loggerSettings[fileLine][fileBufPos] = '\0';
      fileLine++;
    }
    
    settingsCount = fileLine;
   
    // close the file:
    myFile.close();
    //mapp to global settings
    loadGlobalSettings();    
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening RUN file on SD card, cannot run with this, see docs");
    while(true) {      
      connectionOkLight.toggle();
      delay(600);
    }
  }
}
/*
example file on sd card:
;lenr logger settings for run time
SSID=MyWifiNetWork
password=MyPassword
;sensors_enabled piped delimited list,  values are TC1-TC2, Power, Pressure, Light, IR, Geiger
sensors_enabled=TC1|TC2|Power|Pressure|!
plotly-username=username
plotly-password=password
;tokens piped delited list for each sensors_enabled in the same order as speicifed in sensors_enabled
plotly-tokens=dddhfjfj4e|dddhfjfj43|dddhfjf34e|dddhfjfj42|!
plotly-token-count=4
plotly-overwrite=yes
plotly-max-points=300
plotly-filename=test2
disable_sd_logging=no
send_interval_sec=15
*/
String getConfigSetting(char *name) {
  for(int i=0; i<settingsCount; i++) {
    if (getValue(loggerSettings[i], '=', 0).indexOf(name)>-1) {
      return getValue(loggerSettings[i], '=', 1);
    }
  }
  //if name not found then return a null string object
  return String("");
}

int getConfigSettingAsInt(char *name) {  
  if (getConfigSetting(name).length()==0) return 0;
  else return getConfigSetting(name).toInt();
}

float getConfigSettingAsFloat(char *name) {  
  if (getConfigSetting(name).length()==0) return 0.000;
  else return getConfigSetting(name).toFloat();
}

boolean getConfigSettingAsBool(char *name) {  
  return (getConfigSetting(name).indexOf("yes")>-1||getConfigSetting(name).indexOf("true")>-1);
}


boolean isSensorEnabled(char *name) {  
  return (getConfigSetting("sensors_enabled").indexOf(name)>-1);
}

/**
* Save all enabled sensors values to SD card
*/
void saveCsvData()
{
  if (!sdCardFirstLineLogged) {
    saveLineToDatalog(getCsvStringHeaders());
 }
 sdCardFirstLineLogged = true;
 saveLineToDatalog(getCsvString()); 
 
  if (DEBUG_TO_SERIAL == 1) {
      Serial.println("csv data saved to datalog file");
    }
}

