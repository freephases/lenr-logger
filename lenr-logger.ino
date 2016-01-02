/**
* LENR logger
*
* Jan 2016
* Version: 0.0.1.6
*
* Uses:
*  - Arduino Mega ATmega1280
*  _ SD card compatible with SD and SPI libs
*  - MAX31855 with thermocouple x 2 (at least, can have up to 4)
*  - Uno runnning with old style wifi card (rev1) - The wifi slave
*    runs https://github.com/freephases/wifi-plotly-slave
*  - 5v transducer -14.5~30 PSI 0.5-4.5V linear voltage output
*  - Arduino Pro Mini with a OpenEnergyMonitor SMD card using analog ports 0-1 only - the power/emon slave
*    runs https://github.com/freephases/power-serial-slave.git
*  - SSR to control heater power supply (see powerheater tab)
*  - Arduino Pro Mini with Lcd and keypad to display basic values and control PID/SSR run and stop
*    runs https://github.com/freephases/lenr-logger-lcd.git
*
*  Copyright (c) 2015-2016 free phases research
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
#include <SoftwareSerial.h>
/**
* thermocouple driver/amp lib
*/
#include <Adafruit_MAX31855.h>

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
* Send data to wifi slave, can be set in 'RUN' config file on SD card
* allows device to log just to SD card if you so wish
*/
boolean allowDataSend = true;

/**
* Send debug messages to serial 0 not good if using USB connection to PC
*/
boolean debugToSerial = (DEBUG_SLAVE_SERIAL==1);

/**
* Data send interval
* how long do we want the interval between sending data, set via send_interval_sec in config file
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

unsigned long wifiWakeUpMillis = 0;


/**
* Get a plotly token from our piped list in the config file, 0 being the first
*/
String getToken(int index)
{
  return getValue(plotlyTokens, '|', index);
}



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
  Serial.print(" W, ");
  Serial.print(getVrms());
  Serial.println(" Vrms, ");
  Serial.print(getIrms());
  Serial.println(" Irms, ");
}


/**
* Send the data to slave or serial, depending on config settings
*/
void sendData() {
  if (sendDataMillis == 0 || millis() - sendDataMillis >= sendDataInterval) {
    sendDataMillis = millis();

  if (debugToSerial) {
    dumpDebug();
  }
    if (logToSDCard) {
      saveCsvData();
    }

    if (canSendData() && allowDataSend) {
      connectionOkLight.off(); // light is turned on when streaming starts and get an OK from WifiSlave once plot is sent to plotly
      sendPlotlyDataToWifiSlave();
      printRawCsv();
    } else if (!allowDataSend) {
      connectionOkLight.on();
      printRawCsv();
      connectionOkLight.off();
    }
  }
  lcdSlaveSendData();//update LCD display
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
  processPowerSlaveSerial();

  if (allowDataSend) {
    processWifiSlaveSerial();
  }

  //processPowerSlaveSerial();
  processLcdSlaveSerial();
}

/**
* Things to call when not sending data, called by other functions not just loop()
*/
void doMainLoops() {
  manageSerial();
  readSensors();
  powerheaterLoop();
}

/**
* Set up
*/
void setup() {

  Serial.begin(9600); // main serial ports used for debugging or sending raw CSV over USB
  debugSetup();
  
  
  connectionOkLight.on(); //tell the user we are alive

  if (debugToSerial) {
    
    Serial.println("LENR logger"); // send some info on who we are
    Serial.println("");//clear
  }
  
  //set up lcd slave
  setupLcdSlave();
  delay(100);
  lcdSlaveMessage('M', "system  ");
  delay(300);
  //call sd card setup first to load settings. see sdcard.ino file
  lcdSlaveMessage('m', " SD card........");
  sdCardSetup();  
  delay(700);
  //Call our sensors setup funcs  
  lcdSlaveMessage('m', " thermocouples..");
  thermocouple2Setup();
  delay(1000);
  lcdSlaveMessage('m', " pressure.......");
  setupPressure();   
   delay(1000);
  lcdSlaveMessage('m', " power..........");
  powerSlaveSetup();
  
  powerheaterSetup();
 
  delay(3000);

  if (allowDataSend) {
    lcdSlaveMessage('M', "wifi    ");    
    lcdSlaveMessage('m', "plotly stream...");
    delay(20);
    setupWifiSlave();
  }
  sendDataMillis = millis();//set milli secs so we get first reading after set interval
  connectionOkLight.off(); //set light off, will turn on if logging ok, flushing means error
  if (allowDataSend) {
    wifiWakeUpMillis = millis();
    Serial3.println("******");//tell wifi slave we are here
    Serial3.flush();
  } else {
    lcdSlaveMessage('M', "complete  ");
    delay(33);
    lcdSlaveMessage('m', "................");
    delay(1500);
    lcdSlaveMessage('C', "ok");
  }
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
