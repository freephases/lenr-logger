/**
* LENR logger slave interface, my wifi card, not portable very much (YET!)
* Sends data via serial to uno with plotty uplader 
*/
#include "pottysettings.h"

#define ROB_WS_MAX_STRING_DATA_LENGTH 100
int newlineCount = 0;
boolean printWebResponse = false;
int bufferLength = 0;

boolean started = true;

short pos = 0; // position in read buffer
char buffer[ROB_WS_MAX_STRING_DATA_LENGTH+1];
char inByte = 0;
volatile boolean waitingForResponse = false;
volatile boolean theStreamHasStarted = false;
boolean haveConnected = false;
boolean handShakeSucessful = false;

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {
    0, -1        };
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
      found++;
      strIndex[0] = strIndex[1]+1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String getToken(int index)
{
  return getValue(plotlyTokens, '|', index);
}

void processSalveResponse() 
{
  char recordType = buffer[0];
  waitingForResponse = false;  
  switch(recordType) {
  case 'S' : //error
  
     if (DEBUG_TO_SERIAL==1) { Serial.println("slave error"); } 
     //not a lot we can do without any more information about said error so forget about it. Easy!!!
    break;
  case 'O' : //ok - cool, we are still cooking now...
     if (DEBUG_TO_SERIAL==1) { Serial.println("OK"); }
     theStreamHasStarted = true;
     connectionOkLight.on();
    break;
 
  }
}
/**
 * Process data sent vy master
 */
void processSlaveSerial()
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
      startPlotting(overWriteChart);
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
  unsigned long waitStartMillis = millis();
  while(waitingForResponse) {
    doMainLoops();
    if (millis()-waitStartMillis>3000) {      
      waitingForResponse = false; //timeout after 3 secs ;) so we can continue as normal...
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
}

/**
* Send a plot to plotty of int type
*/
void plotByToken(char *token, char *charType, int value) {  
  if (!theStreamHasStarted) {
    return;
  }  
    
  waitForResponse();//wait for other requests to be dealt with
  
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
}

/**
* Send a plot to plotty of float type
*/
void plotByToken(char *token, char *charType, float value) {  
  if (!theStreamHasStarted) {
    return;
  }  
    
  waitForResponse();//wait for other requests to be dealt with, get around with have a single core so a single process thread!!
  
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
}
