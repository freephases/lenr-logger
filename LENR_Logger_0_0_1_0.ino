
/**
* LENR logger
*
* Oct 2015
* Version: 0.0.1.0
*
* Uses:
*  - Arduino Mega ATmega1280
*  - max6675 with thermocouple x 2
*  - Uno runnning SerialToWifiRelay program and old style wifi card (rev1) - The wifi slave
*    runing https://github.com/freephases/wifi-plotly-slave
*  - 5v transducer -14.5~30 PSI 0.5-4.5V linear voltage output
*  - Arduino Pro Mini running OpenEnergyMonitor SMD card using analog ports 0-1 only - the power salve
* running https://github.com/freephases/power-serial-slave.git
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
#define DEBUG_TO_SERIAL 0
//debug raw serial output of the slave if you add jumpers to rx and tx of slave to tx and rx of the mega serial 2 ports
#define DEBUG_SLAVE_SERIAL 0

/**
* Logging option flags:
*/
#define PAD_CSV_SLAVE 1 // flay to use for wifi slave, see https://github.com/freephases/wifi-plotly-slave
#define RAW_CSV 2 // flag for using raw CSV data to serial 0, DEBUG_TO_SERIAL and DEBUG_SLAVE_SERIAL must be set to 0
#define NO_LOGGING 0 //just use debugging mode, be sure to set DEBUG_TO_SERIAL to 1
/**
* Logging option
*/
#define DATA_LOGGERING_MODE PAD_CSV_SLAVE

/**
* Maxium length of data reviced from slaves for 1 record/line
*/
#define MAX_STRING_DATA_LENGTH 150


/**
* Data send interval
* how long do we want the interval between sending data
*/
const unsigned long sendDataInterval = 15000;//send data every XX secs


/**
* global vars for run time, please do not edit
*/
unsigned long sendDataMillis = 0; // last milli secs since sending data to whereever

/**
* Led to signal we are working
*/
OnOff connectionOkLight(30); 

#if DATA_LOGGERING_MODE == RAW_CSV
  /**
  * when in raw CSV mode always send data
  */
  boolean canSendData() {
    return true;
  }
#endif

/**
* Send the data to slave or serial, depending on DATA_LOGGERING_MODE
*/
void sendData() {
  if (canSendData() && (sendDataMillis==0 || millis() - sendDataMillis >= sendDataInterval)) {
    sendDataMillis = millis();
    
#if DEBUG_SLAVE_SERIAL == 1
      //Dump debug info
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
#endif
    
#if DATA_LOGGERING_MODE == PAD_CSV_SLAVE
  connectionOkLight.off(); // light is turned on when streaming starts and get an OK from WifiSlave once plot is sent to plotly
  sendPlotlyDataToWifiSlave();
#else if DATA_LOGGERING_MODE == RAW_CSV
  connectionOkLight.on();
  printRawCsv();
  connectionOkLight.off();  
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

/**
* Process reuired incomming slave serial data
*/
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
  Serial.begin(9600); // main serial ports used for debugging or sending raw CSV over USB
  connectionOkLight.on(); //tell the world we are alive
  if (DEBUG_TO_SERIAL==1) {
    Serial.println("LENR logger"); // send some info on who we are
    Serial.println("");
  }
  
  //Call our sensors setup funcs
  setupPressure();
  powerSlaveSetup();
  
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
#if DEBUG_SLAVE_SERIAL == 1
    processDebugSlaveSerial();// for debug only
#endif
}