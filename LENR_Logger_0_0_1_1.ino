
/**
* LENR logger
*
* Oct 2015
* Version: 0.0.1.1
*
* Uses:
*  - Arduino Mega ATmega1280
*  _ SD card compatible with SD and SPI libs
*  - max6675 with thermocouple x 2 (at least, can have up to 4)
*  - Uno runnning with old style wifi card (rev1) - The wifi slave
*    runing https://github.com/freephases/wifi-plotly-slave
*  - 5v transducer -14.5~30 PSI 0.5-4.5V linear voltage output
*  - Arduino Pro Mini with a OpenEnergyMonitor SMD card using analog ports 0-1 only - the power/emon slave
*    running https://github.com/freephases/power-serial-slave.git
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
* SD card lib + SPI
*/
#include <SPI.h>
#include <SD.h>

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
* Include main defines/settings for logger
*/
#include "logger.h"


/**
* Set complie time logging option - see logger.h logger option flags
*/
#define DATA_LOGGERING_MODE PAD_CSV_SLAVE

/**
* Led to signal we are working, using digital pin 30 on Mega connected to 5v green LED
*/
OnOff connectionOkLight(30);

/**
* global vars for run time...
*/
unsigned long sendDataMillis = 0; // last milli secs since sending data to whereever

/**
* Log data to 'DATALOG' file on SD card - can be disable via RUN file's disable_sd_logging setting
* will just append data to end of file so you need to clean it up before running out of space
*/
boolean logToSDCard = true; 

/**
* Send data to wifi slave or serial depending on DATA_LOGGERING_MODE, can be set in 'RUN' config fiel on SD card
* allows device to log just to SD card if you so wish
*/
boolean allowDataSend = true; 

/**
* Data send interval
* how long do we want the interval between sending data
*/
unsigned long sendDataInterval = 15000;//send data every XX millisecs

/**
* Plotly vars read from config/settings file 'RUN'
*/
char plotlyUserName[30], plotlyPassword[20], plotlyFilename[40], plotlyTokens[70];

/**
* char array to hold a token for ploty when sending plots
*/
char traceToken[11];

/**
* Get a plotly token from our piped list in the config file, 0 being the first
*/
String getToken(int index)
{
  return getValue(plotlyTokens, '|', index);
}


#if DATA_LOGGERING_MODE == RAW_CSV
/**
* when in raw CSV mode always send data to serial
*/
boolean canSendData() {
  return true;
}
#endif

#if DEBUG_TO_SERIAL == 1
/**
* Dump debug info
*/
void dumpDebug() {
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
}
#endif

/**
* Send the data to slave or serial, depending on DATA_LOGGERING_MODE
*/
void sendData() {
  if (sendDataMillis == 0 || millis() - sendDataMillis >= sendDataInterval) {
    sendDataMillis = millis();

#if DEBUG_TO_SERIAL == 1
    dumpDebug();
#endif
    if (logToSDCard) {
      saveCsvData();
    }
    if (canSendData() && allowDataSend) {
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
}

/**
* Read all sensors at given intervals
*/
void readSensors() {
  readThermocouple1();
  readThermocouple2();
  readPressure();
  // readPower();<<moved to mini pro now sent via Serial1

  //todo add more sensor readings....


  // readThermopileArray
  // readGeigerCounter

}

/**
* Process incomming slave serial data
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
  if (DEBUG_TO_SERIAL == 1) {
    Serial.println("LENR logger"); // send some info on who we are
    Serial.println("");
  }

  //call sd card setup first to load settings. see sdcard file
  sdCardSetup();

#if DATA_LOGGERING_MODE == PAD_CSV_SLAVE
  setupWifiSlave();
#endif

  //Call our sensors setup funcs
  setupPressure();
  powerSlaveSetup();

  delay(150);
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
