/**
* LENR logger slave interface, my wifi card, not portable very much (YET!)
* Sends data via serial to uno with plotty uplader
*/

short pos = 0; // position in read buffer
char wifiBuffer[MAX_STRING_DATA_LENGTH + 1];
char inByte = 0;
boolean waitingForResponse = false;
boolean theStreamHasStarted = false;
boolean haveConnected = false;
boolean handShakeSucessful = false;
char wifiSSID[30];//make bigger if you have a long SSID ;)
char wifiPassword[20];//make bigger if you have a long pass ;) but must amend length of buf in wifiSlaveRequestBegin too!
boolean wifiStartRequested = false;
boolean wifiStartRequestSuccess = false;



/**
* Send wifi SSID and password from SD card
*/
void wifiSlaveRequestBegin()
{
  wifiStartRequested = true;
  char buf[60]; //min is wifiSSID+wifiPassword+5
  sprintf(buf, "B|%s|%s|!", wifiSSID,  wifiPassword);
  Serial3.println(buf);
}

/**
* Manage OK responses from slave
*/
void wifiSlaveManageResponse()
{
  if (debugToSerial) {
        Serial.println("OK");
      }
  waitingForResponse = false;

  if (wifiStartRequested) {
    wifiStartRequested = false;
    wifiStartRequestSuccess = true;
    startPlotting();
  } else if (wifiStartRequestSuccess) {
    connectionOkLight.on();
    theStreamHasStarted = true;
    wifiStartRequestSuccess = false;
    //update lcd
    lcdSlaveMessage('M', "complete  ");
    delay(200);
    lcdSlaveMessage('m', "stream started  ");
    delay(1500);
    lcdSlaveMessage('C', "ok");

  } else {
    connectionOkLight.on();
  }
}

/**
* Manage Errors responses from wifi slave
*/
void wifiSlaveManageError()
{
  if (debugToSerial) {
        Serial.print("slave error: ");
        Serial.println(getValue(wifiBuffer, '|', 1));
        
      }
      lcdSlaveMessage('M', "ERROR  *********");
      lcdSlaveError(getValue(wifiBuffer, '|', 1));
      waitingForResponse = false;
}

/**
* Execute actions based on the slaves response
* Record types are E for Error and O for OK, anything else is ignored at the mo 
*/
void processSalveResponse()
{
  char recordType = wifiBuffer[0];  
  
  switch (recordType) {
    case 'E' :  //ERROR WITH REQUEST
      wifiSlaveManageError();
      break;
      
    case 'O' : // REQUEST OK      
      wifiSlaveManageResponse();
      break;
  }
}

/**
* Make sure wifi card serial is working if enabled (i.e wifi option siwtch is off on front pannel)
*/
void checkWifiSerialIsUp()
{
  if (allowDataSend && !handShakeSucessful && wifiWakeUpMillis!=0 && millis()-wifiWakeUpMillis>5000) {
    //wifi slave down
    lcdSlaveMessage('M', "Error***");    
    lcdSlaveMessage('m', "wifi error******");
    delay(4000);
    lcdSlaveMessage('C', "ok");    
    allowDataSend = false;
   // setPowerHeaterAutoMode(false);
    wifiWakeUpMillis = 0;
  }
}

/**
 * Process serial data sent via master from the wifi slave
 */
void processWifiSlaveSerial()
{
  checkWifiSerialIsUp();
  while (Serial3.available() > 0)
  {
    
    // read the incoming byte:
    inByte = Serial3.read();
    if (inByte=='\r') continue;

    if (!haveConnected && inByte == '*') {
      haveConnected = true;
      continue;
    }

    if (!handShakeSucessful && haveConnected && inByte == '\n') {
      handShakeSucessful = true;
    
      if (debugToSerial) {
        Serial.println("Connected to slave");
      }
    
      wifiBuffer[0] = '\0';//clear buffer
      pos = 0;
      wifiSlaveRequestBegin();

      continue;
    }
    if (!handShakeSucessful) {
      continue;
    }
    
    // add to our read buffer
    wifiBuffer[pos] = inByte;
    //   Serial.println(inByte);
    pos++;//increment length
    //Serial.println(inByte);
    if (inByte == '\n' || pos == MAX_STRING_DATA_LENGTH) //end of record or max data length reached
    {
      wifiBuffer[pos - 1] = '\0'; // delimit with 0 char
      processSalveResponse(); //deal with response from slave
      wifiBuffer[0] = '\0'; //Delete wifiBuffer
      pos = 0;
    }

  }
}

/**
* Call main loops, repeated until we get a response from the slave or timeout
*/
boolean waitForResponse(unsigned long defWaitTime=0)
{
  boolean timedOut = false;
  // waitingForResponse = false;
  //  return;
  unsigned long waitStartMillis = millis();
  if (defWaitTime==0) defWaitTime = sendDataInterval-250;
  if (defWaitTime<1000) defWaitTime = 1000;
  while (waitingForResponse) {
    doMainLoops();    
    if (millis() - waitStartMillis > defWaitTime) {
      waitingForResponse = false; //timeout after sendDataInterval-250 so secs ;) so we can continue as normal...
      timedOut = true;
      break;
    }
  }
  
  return timedOut;
}

/**
* Initialise a stream with plotly via the slave, required before we can start streaming
*/
void startPlotting() {
  char buf[120];//wifi slave has limit of 120 chars

  //build request for slave
  sprintf(buf, "S|%s|%s|%s|%d|%d|%d|%s|!",
          plotlyUserName,
          plotlyPassword,
          plotlyFilename,
          getConfigSettingAsBool("plotly-overwrite") ? 1 : 0,
          getConfigSettingAsInt("plotly-max-points"),
          getConfigSettingAsInt("plotly-token-count"),
          plotlyTokens);

  Serial3.println(buf);//send request
  waitingForResponse = true; //note we are waiting for a response before we do another request
  if (debugToSerial) {
    Serial.println(buf);
    Serial.println("Sent start plotting command");
  }
  if (waitForResponse(90000)) {
     startPlotting();
  }
}

/**
* Send a plot to plotly of int type
*/
void plotByToken(char *token, int value) {
  if (!theStreamHasStarted) {
    return;
  }
 
  char buf[70];
  String v = String(value);
  char intStr[15];
  v.toCharArray(intStr, 15);
  sprintf(buf, "P|%s|%s|%s|!", token, "I", intStr);// build up request

  if (debugToSerial) {
    Serial.println(buf);
  }

  Serial3.println(buf);// send request
 
  waitingForResponse = true; // note that we sent a request and expect a response before sending more stuff
  waitForResponse();
}

/**
* Send a plot to plotly of float type
*/
void plotByToken(char *token, float value) {
  if (!theStreamHasStarted) {
    return;
  }

  //waitForResponse();//wait for other requests to be dealt with, get around with have a single core so a single process thread!!

  String tempStr = String(value);//map float to a string
  char floatBuf[15];
  tempStr.toCharArray(floatBuf, 15);//map float to char array

  char buf[100];
  sprintf(buf, "P|%s|%s|%s|!", token, "F", floatBuf); // build up request

 
  Serial3.println(buf);// send request

  waitingForResponse = true; // note that we sent a request and expect a response before sending more stuff
  waitForResponse();
}

/**
* Retirns true if the stream has started
*/
boolean canSendData() {
  return theStreamHasStarted;
}

/**
* Returns true if still waiting for a respose from the slave
*/
boolean isWaitingForResponse() {
  return waitingForResponse;
}

/**
* Send request to wifi slave to send request to plotly
*/
void sendPlotlyDataToWifiSlave() {
  int tokenToUse = 0; // increment enabled sensors to match list of plotly_tokens, no error checking yet so need make sure run file on sd card is correct
  //Send temp
  //  ..core
  if (isSensorEnabled("TC1")) {
    getToken(tokenToUse).toCharArray(traceToken, 11);//select token
    plotByToken(traceToken, getThermocoupleAvgCelsius1());//send token with our value
   // delay(120);
    tokenToUse++;
  }
  //  ..room
  if (isSensorEnabled("TC2")) {
    getToken(tokenToUse).toCharArray(traceToken, 11);
    plotByToken(traceToken, getThermocoupleAvgCelsius2());
  //  delay(120);
    tokenToUse++;
  }
  //Send power
  if (displayDCPower) {
    getToken(tokenToUse).toCharArray(traceToken, 11);
    plotByToken(traceToken, getHbridgeWatts());
    tokenToUse++;
  } else if (isSensorEnabled("Power")) {
    getToken(tokenToUse).toCharArray(traceToken, 11);
    plotByToken(traceToken, getPower());
  //  delay(120);
    tokenToUse++;
  }

  //Send PSI
  if (isSensorEnabled("Pressure")) {
    getToken(tokenToUse).toCharArray(traceToken, 11);
    plotByToken(traceToken, getPressurePsi());
    
    //tokenToUse++;
  }
  
   

}

/**
* Setup -  do stuff before the dance begins (shame i'll never know when the fat lady will sing or do anything when it happens!)
*/
void setupWifiSlave() {
  Serial3.begin(9600);//slaves serial
  
  getConfigSetting("SSID").toCharArray(wifiSSID, 30);
  getConfigSetting("password").toCharArray(wifiPassword, 20);
    
}

/**
* Returns true if we have valid and active plotly
*/
boolean getTheStreamHasStarted() {
  return theStreamHasStarted;
}


