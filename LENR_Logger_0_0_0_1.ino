
/**
* LENR logger
*
* Oct 2015
* Version: 0.0.0.1-alpha-blue-pants - NOT COMPLETE YET, W.I.P.W.D.Y.E!
*
* Uses:
*  Arduino Mega ATmega1280
*  max6675 with thermocouple x 2
*  Uno runnning SerialToWifiRelay program and old style wifi card (rev1) - THE wifi SLAVE
*  5v transducer -14.5~30 PSI 0.5-4.5V linear voltage output
*  mini pro running OpenEnergyMonitor SMD card using analog ports 0-1 only - the power salve
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
* thermocouple driver/amp lib
*/
#include "max6675.h"

/**
* OnOff is simple class to manage digital ports see: https://github.com/freephases/arduino-onoff-lib
*/
#include <OnOff.h>


//#include <TimerOne.h> //to do
/**
* Settings
**/
// send debug info to serial - 1 is on anything else is off
#define DEBUG_TO_SERIAL 1
//debug raw serial output of the slave if you add jumpers to rx and tx od slave to tx and rx of the mega
#define DEBUG_SLAVE_SERIAL 0

//use a slave device able to read CSV data or set to 0 to use python example format TODO
#define PAD_CSV_SLAVE 1 // use wifi slave
#define RAW_CSV 2 // send raw CSV data to serial 0, DEBUG_TO_SERIAL and DEBUG_SLAVE_SERIAL must be 0

#define DATA_LOGGERING_MODE PAD_CSV_SLAVE

#define ROB_WS_MAX_STRING_DATA_LENGTH 150
//how long do we want the interval between sending data to plotly to be
const unsigned long sendDataInterval = 15000;//send data every 30 secs

// see other files in tap above for specific sensor settings and mnay other things

/**
* global vars for run time, please do not edit
*/
unsigned long sendDataMillis = 0; // last milli secs since sending data to whereever


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
* Return a value with in a CSV string where index is the 
*/
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

/**
* Send the traces to our plotly stream
*/
void sendData() {
  if (canSendData && (sendDataMillis==0 || millis() - sendDataMillis >= sendDataInterval)) {
    sendDataMillis = millis();
    connectionOkLight.off();

    if (DEBUG_TO_SERIAL == 1) {
      Serial.print("Reactor core temp: ");
      Serial.print(getThermocoupleAvgCelsius1());
      Serial.println(" Celsius");
      Serial.print("Room temp: ");
      Serial.print(getThermocoupleAvgCelsius2());
      Serial.println(" Celsius");
      Serial.print("Reactor pressure: ");
      Serial.print(getPressurePsi());
      Serial.println(" PSI");
      Serial.print("Power: ");
      Serial.print(getPower());
      Serial.println("W");
    }
    
#if DATA_LOGGERING_MODE == PAD_CSV_SLAVE
  sendPlotlyDataToWifiSlave();
#endif
#if DATA_LOGGERING_MODE == RAW_CSV
  printRawCsv();
#endif
        
  }
}

/**
* Read all sensors at given intervals
*/
void readSensors() {
  readThermocouple1();
  readThermocouple2();
  readPressure();
 // readPower();<<move to mini pro now sent via Serial1
  
  //todo add more sensor readings....

 
  // readThermopileArray
  // readGeigerCounter
 
}

void manageSerial() {
#if DATA_LOGGERING_MODE == PAD_CSV_SLAVE  
  processWifiSlaveSerial();
#endif
  processPowerSlaveSerial();
}

/**
* Things to call when not sending data, called by other functions not just loop()
*/
void doMainLoops() {  
  manageSerial();
  readSensors();
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
  powerSlaveSetup();
  
  //lcd.begin(16, 2);
  //printToLcdWithDelay(0, "Starting...  ");
  //printToLcdWithDelay(1, "     ...  ");
  
#if DATA_LOGGERING_MODE == PAD_CSV_SLAVE  
    setupWifiSlave();
#endif

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
