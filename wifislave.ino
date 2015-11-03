/**
* LENR logger slave interface, my wifi card, not portable very much (YET!)
* Sends data via serial to uno with plotty uplader
*/

#if DATA_LOGGERING_MODE == PAD_CSV_SLAVE

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
* Execute actions based on the slaves response
* Record types are E for Error and O for OK, anything else is ignored
*/
void processSalveResponse()
{
  char recordType = wifiBuffer[0];

  switch (recordType) {
    case 'E' :  //ERROR WITH REQUEST
      if (DEBUG_TO_SERIAL == 1) {
        Serial.print("slave error: ");
        Serial.println(getValue(wifiBuffer, '|', 1));
      }
      waitingForResponse = false;
      break;
      
    case 'O' : // REQUEST OK
      if (DEBUG_TO_SERIAL == 1) {
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
      } else {
        connectionOkLight.on();
      }
      break;
  }
}

/**
 * Process serial data sent via master from the wifi slave
 */
void processWifiSlaveSerial()
{
  while (Serial3.available() > 0)
  {
    // read the incoming byte:
    inByte = Serial3.read();
    //if (inByte=='\r') continue;

    if (!haveConnected && inByte == '*') {
      haveConnected = true;
      continue;
    }

    if (!handShakeSucessful && haveConnected && inByte == '\n') {
      handShakeSucessful = true;

      if (DEBUG_TO_SERIAL == 1) {
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
void waitForResponse()
{
  // waitingForResponse = false;
  //  return;
  unsigned long waitStartMillis = millis();
  while (waitingForResponse) {
    doMainLoops();
    if (DEBUG_SLAVE_SERIAL == 1) {
      processDebugSlaveSerial();// for debug only
    }
    if (millis() - waitStartMillis > 10000) {
      //      waitingForResponse = false; //timeout after 80 so secs ;) so we can continue as normal...
      break;
    }
  }
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
  if (DEBUG_TO_SERIAL == 1) {
    Serial.println(buf);
    Serial.println("Sent start plotting command");
  }
  waitForResponse();
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

  if (DEBUG_TO_SERIAL == 1) {
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

boolean canSendData() {
  return theStreamHasStarted;
}

boolean isWaitingForResponse() {
  return waitingForResponse;
}

void sendPlotlyDataToWifiSlave() {
  int tokenToUse = 0; // increment enabled sensors to match list of plotly_tokens, no error checking yet so need make sure run file on sd card is correct
  //Send temp
  //  ..core
  if (isSensorEnabled("TC1")) {
    getToken(tokenToUse).toCharArray(traceToken, 11);//select token
    plotByToken(traceToken, getThermocoupleAvgCelsius1());//send token with our value
    delay(120);
    tokenToUse++;
  }
  //  ..room
  if (isSensorEnabled("TC2")) {
    getToken(tokenToUse).toCharArray(traceToken, 11);
    plotByToken(traceToken, getThermocoupleAvgCelsius2());
    delay(120);
    tokenToUse++;
  }
  //Send power
  if (isSensorEnabled("Power")) {
    getToken(tokenToUse).toCharArray(traceToken, 11);
    plotByToken(traceToken, getPower());
    delay(120);
    tokenToUse++;
  }

  //Send PSI
  if (isSensorEnabled("Pressure")) {
    getToken(tokenToUse).toCharArray(traceToken, 11);
    plotByToken(traceToken, getPressurePsi());
    
    //tokenToUse++;
  }
  
   

}


void setupWifiSlave() {
  Serial3.begin(9600);//slaves serial
  Serial2.begin(9600);//feed back slaves main serial if you wire up 0 and 1 back to mega serial 2 ports
  delay(500);
  getConfigSetting("SSID").toCharArray(wifiSSID, 30);
  getConfigSetting("password").toCharArray(wifiPassword, 20);
  Serial3.println("**************");//tell wifi slave we are here
}

#endif
