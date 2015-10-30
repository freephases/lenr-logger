
/**
* LENR logger
*
* Oct 2015
* Version: 0.0.0.1-alpha-blue-pants - NOT COMPLETE YET, W.I.P.W.D.Y.E!
*
* Uses:
*  Arduino Mega ATmega1280
*  max6675 with thermocouple or more
*  Uno runnning SerialToWifiRelay program and old style wifi card (rev1) - THE SLAVE
*  5v transducer -14.5~30 PSI 0.5-4.5V linear voltage output
*  OpenEnergyMonitor SMD card using analog ports 0-1 only for I and V linked via the mega by the wonders of wires (not pinz)
*  
*  
*  Copyright (c) 2015 free phases
*  
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*  
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*  
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

/**
* thermocouple amp lib
*/
#include "max6675.h"
#include <EmonLib.h>
//#include <LiquidCrystal.h>
//also todo: SD card reader to get WiFi and Plotly passwords
#include <OnOff.h>
//#include <TimerOne.h> cannot use as mothods require a lot of work.
/**
* Settings
**/
// send debug info to serial - 1 is on anything else is off
#define DEBUG_TO_SERIAL 0
//debug raw serial output of the slave if you add jumpers to rx and tx od slave to tx and rx of the mega
#define DEBUG_SLAVE_SERIAL 0

//use a slave device able to read CSV data or set to 0 to use python example format TODO
#define PAD_CSV_SLAVE 1
#define PLYTHON_SERIAL 2
#define EXCEL 3
#define DATA_LOGGERING_MODE PAD_CSV_SLAVE

//Plotly tokens index for each trace
#define TRACE_CORE_TEMP   0
#define TRACE_ROOM_TEMP   1
#define TRACE_POWER       2
#define TRACE_PRESSURE    3

//how long do we want the interval between sending data to plotly to be
const long sendDataInterval = 30000;//send data every 30 secs

// see other files in tap above for specific sensor settings and mnay other things

/**
* global vars for run time, please do not edit
*/
unsigned long sendDataMillis = 0; // last milli secs since sending data to whereever

char traceToken[11]; //char array to hold a token for ploty

OnOff connectionOkLight(30); // nice light so we know it's working

//lcd - cannot get to work with my mega so left for now...
/*LiquidCrystal lcd(7, 6, 5, 4, 3, 2);


void printToLcdWithDelay(byte lineNum, char* message, int delayTime = 350) {
  lcd.setCursor(0, lineNum);
  lcd.print(message);
  lcd.print("                 ");// clear line
  delay(delayTime);
}*/

/**
* Send the traces to our plotly stream
*/
void sendData() {
  if (getThermocoupleAvgCelsius1() > -200.0000 && millis() - sendDataMillis >= sendDataInterval) {
    sendDataMillis = millis();
    connectionOkLight.off();

    if (DEBUG_TO_SERIAL == 1) {
      Serial.print("Reactor core temp: ");
      Serial.print(getThermocoupleAvgCelsius1());
      Serial.println(" Celsius");
      Serial.print("Reactor pressure: ");
      Serial.print(getPressurePsi());
      Serial.println(" PSI");
      Serial.print("Power: ");
      Serial.print(getPower());
      Serial.println("W");
    }
    
    //Send temp
    //  ..core
    getToken(TRACE_CORE_TEMP).toCharArray(traceToken, 11);//select token
    plotByToken(traceToken, "F", getThermocoupleAvgCelsius1());//send token with our value
    //  ..room
    getToken(TRACE_ROOM_TEMP).toCharArray(traceToken, 11);
    plotByToken(traceToken, "F", getThermocoupleAvgCelsius2());
        
    //Send power
    getToken(TRACE_POWER).toCharArray(traceToken, 11);
    plotByToken(traceToken, "F", getPower());
    
    //Send PSI
    getToken(TRACE_PRESSURE).toCharArray(traceToken, 11);
    plotByToken(traceToken, "I", getPressurePsi());
    
    // see: https://plot.ly/streaming/
    //    and https://github.com/plotly/arduino-api
  }
}

/**
* Read all sensors at given intervals
*/
void readSensors() {
  readThermocouple1();
  readThermocouple2();
  readPressure();
  readPower();
  
  //todo add more sensor readings....

 
  // readThermopileArray
  // readGeigerCounter
 
}

/**
* Things to call when not sending data, called by other functions not just loop()
*/
void doMainLoops() {
  readSensors();
  switch (DATA_LOGGERING_MODE) {
    case PAD_CSV_SLAVE: processSlaveSerial();
      break;
    //case PLYTHON_SERIAL:
    //case EXCEL:
    //case TO_SDCARD_AS_CSV:
    default:
      Serial.println("ERORR: DATA_LOGGERING_MODE NOT COMPLETED YET");
      break;
  }
}

/**
* Set up
*/
void setup() {
  Serial.begin(9600); // main serial ports used for debugging only so far
  connectionOkLight.on(); //tell the world i'm alive
  Serial.println("LENR logger"); // send some info on who we are
  Serial.println(""); // clear a line for connecting serial in case we are doig that sort of thing ;)
  
  //Call our sensors setup funcs
  setupPressure();
  setupPower();
  
  //lcd.begin(16, 2);
  //printToLcdWithDelay(0, "Starting...  ");
  //printToLcdWithDelay(1, "     ...  ");
  
  //Prepare slaves and masters as required
  if (DATA_LOGGERING_MODE == PAD_CSV_SLAVE) {
    Serial3.begin(9600);//slaves serial
    Serial2.begin(9600);//salve feed back if you wire up 0 and 1 back to mega serial 2 ports
    //printToLcdWithDelay(1, "Waiting for wifi");
  } else {
     //printToLcdWithDelay(1, "Not a complete mode - todo");
  }
  delay(50);
  sendDataMillis = millis();//set milli secs so we get first reading after set interval
  connectionOkLight.off();
}

/**
* Main loop
*/
void loop() {
  doMainLoops();
  sendData();
  if (DEBUG_SLAVE_SERIAL == 1) {
    processDebugSlaveSerial();// for debug only
  }
}
