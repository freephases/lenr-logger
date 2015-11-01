/**
* LENR logger slave interface, my wifi card, not portable very much (YET!)
* Sends data via serial to uno with plotty uplader 
*/

#if DATA_LOGGERING_MODE == PAD_CSV_SLAVE
#include "plotlysettings.h"

short pos = 0; // position in read buffer
char buffer[ROB_WS_MAX_STRING_DATA_LENGTH+1];
char inByte = 0;
boolean waitingForResponse = false;
boolean theStreamHasStarted = false;
boolean haveConnected = false;
boolean handShakeSucessful = false;
char traceToken[11]; //char array to hold a token for ploty


String getToken(int index)
{
  return getValue(plotlyTokens, '|', index);
}

void processSalveResponse() 
{
  char recordType = buffer[0];
 
  switch(recordType) {
  case 'S' : //error
  
     if (DEBUG_TO_SERIAL==1) { Serial.println("slave error"); } 
     //not a lot we can do without any more information about said error so forget about it. Easy!!!
    break;
  case 'O' : //ok - 
     if (DEBUG_TO_SERIAL==1) { Serial.println("OK"); }
     theStreamHasStarted = true;
     connectionOkLight.on();
      waitingForResponse = false;  
    break;
 
  }
}
/**
 * Process data sent vy master
 */
void processWifiSlaveSerial()
{
  // send data only when you receive data:
  while (Serial3.available() > 0)
  {

    // read the incoming byte:
    inByte = Serial3.read();
    if (!haveConnected && inByte == '*') {
      haveConnected = true;           
      continue;
    } 

    if (!handShakeSucessful && haveConnected && inByte == '\n') {
      handShakeSucessful = true;
      if (DEBUG_TO_SERIAL==1) { 
        Serial.println("Connected to slave");
      }
      Serial3.println("*");
      delay(300);
      buffer[0] = 0; 
      pos = 0;   
      startPlotting(overWriteChart);
     // processSlaveSerial();
      continue;
    }
    if (!handShakeSucessful) {
      continue;
    }

  
    // add to our read buffer
    buffer[pos] = inByte;
    //   Serial.println(inByte); 
    pos++;
    //Serial.println(inByte);
    if (inByte == '\n' || pos==ROB_WS_MAX_STRING_DATA_LENGTH) //end of max field length
    {
      buffer[pos-1] = 0; // delimited      
      processSalveResponse(); 
      buffer[0] = '\0';
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
  while(waitingForResponse) {
    doMainLoops();
    if (DEBUG_SLAVE_SERIAL == 1) {
      processDebugSlaveSerial();// for debug only
    }
    if (millis()-waitStartMillis>10000) {      
//      waitingForResponse = false; //timeout after 80 so secs ;) so we can continue as normal...
      break;
    }    
  }
}

/**
* Initialise a stream with plotly via the slave, required before we can start streaming
*/
void startPlotting(int overWrite) {    
  char buf[100];
  //build request for slave
  sprintf(buf, "S|%s|%s|%s|%d|%d|%d|%s|", 
    plotlyUserName, 
    plotlyPassword,
    plotlyFilename,
    overWrite,    
    plotlyMaxPoints,
    plotlyTokenCount,
    plotlyTokens);
            
  Serial3.println(buf);//send request 
  waitingForResponse = true; //note we are waiting for a response before we do another request
  if (DEBUG_TO_SERIAL==1) {
    Serial.println(buf);
    Serial.println("Sent start plotting command");
  }
   waitForResponse();
}

/**
* Send a plot to plotty of int type
*/
void plotByToken(char *token, char *charType, int value) {  
  if (!theStreamHasStarted) {
    return;
  }  
    
 // waitForResponse();//wait for other requests to be dealt with
  
  char buf[100];
  sprintf(buf, "P|%s|%s|%d|", token, charType, value);// build up request
  
  if (DEBUG_TO_SERIAL==1) {
    Serial.println(buf);
  }
  
  Serial3.println(buf);// send request
  waitingForResponse = true; // note that we sent a request and expect a response before sending more stuff
  
  if (DEBUG_TO_SERIAL==1) {
    Serial.println("Plot sent");
  }
   waitForResponse();
}

/**
* Send a plot to plotty of float type
*/
void plotByToken(char *token, char *charType, float value) {  
  if (!theStreamHasStarted) {
    return;
  }  
    
  //waitForResponse();//wait for other requests to be dealt with, get around with have a single core so a single process thread!!
  
  String tempStr = String(value);//map float to a string
  char floatBuf[16];
  tempStr.toCharArray(floatBuf, 16);//map float to char array
  
  char buf[100];
  sprintf(buf, "P|%s|%s|%s|", token, charType, floatBuf); // build up request
  
  if (DEBUG_TO_SERIAL==1) {
    Serial.println(buf);
  }
  
  Serial3.println(buf);// send request
  waitingForResponse = true; // note that we sent a request and expect a response before sending more stuff
  
  if (DEBUG_TO_SERIAL==1) {
    Serial.println("Plot sent");
  }
  
   waitForResponse();
}

boolean canSendData() {
  return theStreamHasStarted;
}

boolean isWaitingForResponse() {
  return waitingForResponse;
}

void sendPlotlyDataToWifiSlave() {
  //Send temp
    //  ..core
    getToken(TRACE_CORE_TEMP).toCharArray(traceToken, 11);//select token
    plotByToken(traceToken, "F", getThermocoupleAvgCelsius1());//send token with our value
    delay(120);
    //  ..room
    getToken(TRACE_ROOM_TEMP).toCharArray(traceToken, 11);
    plotByToken(traceToken, "F", getThermocoupleAvgCelsius2());
    delay(120);    
    //Send power
    getToken(TRACE_POWER).toCharArray(traceToken, 11);
    plotByToken(traceToken, "F", getPower());
    delay(120);
    //Send PSI
    getToken(TRACE_PRESSURE).toCharArray(traceToken, 11);
    plotByToken(traceToken, "I", getPressurePsi());
   
    // see: https://plot.ly/streaming/
    //    and https://github.com/plotly/arduino-api
}

void setupWifiSlave() {
  Serial3.begin(9600);//slaves serial
    Serial2.begin(9600);//salve feed back if you wire up 0 and 1 back to mega serial 2 ports
}

#endif
