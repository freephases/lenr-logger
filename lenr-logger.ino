#include <PID_v1.h>

#include <RunningMedian.h>

/**
* LENR logger
*
*
* Uses:
*  - Arduino Mega ATmega1280
*  _ SD card compatible with SD and SPI libs
*  - MAX31855 with thermocouple x 2 (at least, can have up to 4)
*  - Uno runnning with old style wifi card (rev1) - The wifi slave
*    runs https://github.com/freephases/wifi-plotly-slave
*  - 5v transducer -14.5~30 PSI 0.5-4.5V linear voltage output
*  - SSR to control heater power supply (see powerheater tab)
*  - Arduino Pro Mini with Lcd and keypad to display basic values and control PID/SSR run and stop
*    runs https://github.com/freephases/lenr-logger-lcd.git
*  - GC-10 added as new default option
*  - OPTIONAL: Arduino Pro Mini with a OpenEnergyMonitor SMD card using analog ports 0-1 only - the power/emon slave
*    runs https://github.com/freephases/power-serial-slave.git
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

#include <math.h>
/**
* thermocouple driver/amp lib
*/
#include <Adafruit_MAX31855.h>

/**
* OnOff is simple class to manage digital ports see: https://github.com/freephases/arduino-onoff-lib
*/
#include <OnOff.h>

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
boolean debugToSerial = (DEBUG_SLAVE_SERIAL == 1);

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
* Send control commands to h-bridge as well as default SSR (can unplug SSR if not needed)
*/
boolean controlHbridge = true;

/**
* Thermocouple DO and CLK ports, same for all thermocouples
*/
const int thermoDO = 3; //same as TC2
const int thermoCLK = 5; //same as TC2

/**
* Thermocouple offsets, set by sd card if seting exists, see config
*/
int thermocoupleOffSet1 = 2.98;
int thermocoupleOffSet2 = 3.08;

/**
* hBridgeSpeed speed, set by sd card if seting exists, see config or
* by lcd/keypad via lcdslave
*/
int hBridgeSpeed = 75;//~3600Hz which is about 500 pluses with AC at 1000Hz; more work todo

float calibratedVoltage = 4.976; //was 4.980 before 2016-01-03; now set by sd card if setting exists

/**
* Get a plotly token from our piped list in the config file, 0 being the first
*/
String getToken(int index)
{
  return getValue(plotlyTokens, '|', index);
}


/**
* Send the data to slave or serial, depending on config settings
*/
void sendData() {
  if (sendDataMillis == 0 || millis() - sendDataMillis >= sendDataInterval) {
    sendDataMillis = millis();

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
  readPressure();
  readThermocouple2();
  //todo add more sensor readings....
  // readGeigerCounter

}

short serial1BufferPos = 0; // position in read buffer
char serial1Buffer[MAX_STRING_DATA_LENGTH_SMALL + 1];
char serial1BufferInByte = 0;
/**
 * Process serial data sent via serial 1 uses end of line as delimiter
 */
void processSerial1()
{
  while (Serial1.available() > 0)
  {
    serial1BufferInByte = Serial1.read();

    // add to our read buffer
    serial1Buffer[serial1BufferPos] = serial1BufferInByte;
    serial1BufferPos++;

    if (serial1BufferInByte == '\n' || serial1BufferPos == MAX_STRING_DATA_LENGTH) //end of max field length
    {
      serial1Buffer[serial1BufferPos - 1] = 0; // delimited
      processSerial1Response();
      serial1Buffer[0] = '\0';
      serial1BufferPos = 0;
    }
  }
}

/**
* Process incomming slave serial data
*/
void manageSerial() {
  processSerial1();

  if (allowDataSend) {
    processWifiSlaveSerial();
  }

  processLcdSlaveSerial();
  processHbridgeSerial();
}

/**
* Things to call when not sending data, called by other functions not just loop()
*/
void doMainLoops()
{
  manageSerial();
  readSensors();
  powerheaterLoop();
}

/**
* Sensor and control components set up
**/
void setupDevices()
{
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
  delay(1000); // wait for MAX chips to stabilize
  lcdSlaveMessage('m', " pressure.......");
  setupPressure();
  delay(1000);
  Serial1.begin(9600);
#if (SERIAL1_USAGE == S1_EMON)
  lcdSlaveMessage('m', " power..........");
#elseif (SERIAL1_USAGE == S1_GC10)
  lcdSlaveMessage('m', " geiger conter..");
#endif
  delay(1000);
  lcdSlaveMessage('m', " SSR............");
  powerheaterSetup();
  delay(500);
  lcdSlaveMessage('m', " H-Bridge.......");
  hBridgeSetup();
  delay(1000);

  //display connent to wifi if enabled
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
    //ready to go as no internet required or availible
    lcdSlaveMessage('M', "complete  ");
    delay(33);
    lcdSlaveMessage('m', "................");
    delay(1500);
    lcdSlaveMessage('C', "ok");
  }
}

/**
* Set up
*/
void setup()
{
  Serial.begin(9600); // main serial ports used for debugging or sending raw CSV over USB for PC processing
  connectionOkLight.on(); //tell the user we are alive
  Serial.println("");//clear
  Serial.println("");
  Serial.print("i|LENR logger v"); // send some info on who we are, i = info, using pipes for delimiters, cool man!
  Serial.print(getVersion());
  Serial.println(";");
  setupDevices();
  Serial.println("i|started");
}

/**
* Main loop
*/
void loop() {
  doMainLoops();
  sendData();
}
