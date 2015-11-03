/**
LENR logger

Basic methods to load config settings from SD card

Example Settings file contents is...

;lenr logger settings for run time
SSID=MyWifiNetWork
password=MyPassword
;sensors_enabled piped delimited list,  values are TC1-TC2, Power, Pressure, Light, IR, Geiger
sensors_enabled=TC1|TC2|Power|Pressure|!
plotly-username=username
plotly-password=password
;tokens piped delimited list for each sensors_enabled in the same order as speicifed in sensors_enabled
plotly-tokens=dddhfjfj4e|dddhfjfj43|dddhfjf34e|dddhfjfj42
plotly-token-count=4
plotly-overwrite=yes
plotly-max-points=300
plotly-filename=your_filename_here
disable_sd_logging=no
send_interval_sec=15

*/

/**
* Max number of setting we can have in the config file
*/
#define MAX_SETTINGS 14

/**
* Char array to hold each line of the config file, we ignore lines starting with ';' but not empty lines yet!!
*/
char loggerSettings[MAX_SETTINGS][70]= {"", "", "", "", "", "", "", "", "", "", "", "", "",""};

/**
* Total number of settings loaded
*/
int settingsCount = 0;

/**
*Load config file from SD card
*/
void loadConfig()
{
  // re-open the file for reading:
  File myFile = SD.open("run.txt");//file must not have txt ext on device - not sure why
  if (myFile) {
    if (DEBUG_TO_SERIAL == 1) {
      Serial.println("opened file to read run.txt:");
    }
    
    byte fileChar;
    int fileBufPos = 0;
    int fileLine = 0;
    char fileBuf[70];
    boolean ignoreUntilNewLine = false;
    
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
    Serial.println("error opening RUN file on SD card, cannot run without this, see the doc");
    while(true) {      
      connectionOkLight.toggle();
      delay(600);
    }
  }
}

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

