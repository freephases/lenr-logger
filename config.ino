/**
LENR logger

Basic methods to load config settings from SD card

Example Settings file contents is...

;lenr logger settings for run time
SSID=MyWifiNetWork
password=MyPassword
;sensors_enabled piped delimited list,  values are TC1-TC2, Power, Pressure, Light, IR, Geiger
sensors_enabled=TC1|TC2|Power|Pressure|Geiger|!
plotly-username=username
plotly-password=password
;tokens piped delimited list for each sensors_enabled in the same order as speicifed in sensors_enabled
plotly-tokens=dddhfjfj4e|dddhfjfj43|dddhfjf34e|dddhfjfj42|ffee8sjgg
plotly-token-count=5
plotly-overwrite=yes
plotly-max-points=3000
plotly-filename=your_filename_here
disable_sd_logging=no
send_interval_sec=5
disable_data_send=no
;PID programs
;csv list of values for each program overstep
power_oversteps=5.5,10.5
;csv list of values for each program off temp
power_off_temp=1000.5,399.5
;csv list of values for each program length in minutes
run_time_mins=120,60
;repeat all programs above, 0 = off,
repeat=0
debug_to_serial=no
;switch ac with SSR or no to use DC and h-bridge
switch_ac=no
;hbridge_speed percentage 100=max, 0=min
hbridge_speed=75
;thermocouple offsets
tc_offsets=0.00,0.49
cal_voltage=4.957
;lowestPowerPercentage
lowest_power=77
*/

/**
* Max number of setting we can have in the config file
*/
#define MAX_SETTINGS 26

/**
* Char array to hold each line of the config file, we ignore lines starting with ';' 
*/
char loggerSettings[MAX_SETTINGS][70]= {"", "", "", "", "", "", "", "", "", "", "", "", "","","","","","","","","","","","","",""};

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
    if (debugToSerial) {
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
    /*if (fileBufPos!=0 && fileLine<10) {
      loggerSettings[fileLine][fileBufPos] = '\0';
      fileLine++;
    }*/
    
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

int getConfigSettingAsInt(char *name, int defaultVal=0) {  
  if (getConfigSetting(name).length()==0) return defaultVal;
  else return getConfigSetting(name).toInt();
}

float getConfigSettingAsFloat(char *name, int defaultVal=0.000) {  
  if (getConfigSetting(name).length()==0) return defaultVal;
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
  allowDataSend = !getConfigSettingAsBool("disable_data_send");
  if (DEBUG_SLAVE_SERIAL==0) {
    debugToSerial = getConfigSettingAsBool("debug_to_serial");
  } 
  
  controlHbridge = (getConfigSettingAsBool("switch_ac")==false);
  
  thermocoupleEnabledCount = getConfigSettingAsInt("tc_enabled_count", thermocoupleEnabledCount);

  unsigned long sI = getConfigSettingAsInt("send_interval_sec");
  if (sI<1) sI = defaultSendDataIntervalSecs;
  sendDataInterval = sI*1000;
  
  if (getConfigSettingAsInt("hbridge_speed",-1)!=-1) {
    hBridgeSpeed = getConfigSettingAsInt("hbridge_speed");
    if (hBridgeSpeed>100) {//tmp force while testing to 93 - about 190 hz AC sq wave
      hBridgeSpeed = 100;
    } else if (hBridgeSpeed<0) {
      hBridgeSpeed = 0;
    }
  }
  
  if (getConfigSettingAsFloat("max_temp",-999.999)!=-999.999) {
    manualMaxTemp = getConfigSettingAsFloat("max_temp");
  }
    
   
  if (getConfigSettingAsFloat("cal_voltage",-999.999)!=-999.999) {
    calibratedVoltage = getConfigSettingAsFloat("cal_voltage");
  }

  
 
  
  
  
  
}

