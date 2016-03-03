#ifndef __LOGGER_H__
#define __LOGGER_H__

/**
* debug raw serial output of the slave if you add jumpers to rx and tx of slave to tx and rx of the mega serial 2 ports
*/
#define DEBUG_SLAVE_SERIAL 0

/**
* Logging option flags:

#define PAD_CSV_SLAVE 1001 // flay to use for wifi slave, see https://github.com/freephases/wifi-plotly-slave
#define RAW_CSV 1002 // flag for using raw CSV data to serial 0, DEBUG_TO_SERIAL and DEBUG_SLAVE_SERIAL must be set to 0
#define NO_LOGGING 1000 //just use debugging mode, be sure to set DEBUG_TO_SERIAL to 1
*/

/**
* Maxium length of data reviced from slaves for 1 record/line
*/
#define MAX_STRING_DATA_LENGTH 160

/**
* Default send interval when setting 'send_interval_sec' is missing from the 'RUN' file on SD card
*/
const unsigned long defaultSendDataIntervalSecs = 15;


#endif
