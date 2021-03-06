#ifndef __LOGGER_H__
#define __LOGGER_H__

/**
* debug raw serial output of the slave if you add jumpers to rx and tx of slave to tx and rx of the mega serial 2 ports
*/
#define DEBUG_SLAVE_SERIAL 0



/**
* Defines for SERIAL1_USAGE, geiger counter or emon client, geiger counter is new default option
*/
#define S1_GC10 1 // GC10 geiger counter client
#define S1_EMON 2 // emon client
/**
* SERIAL1_USAGE
*/
#define SERIAL1_USAGE S1_GC10

/**
* LL_TC_USE_AVG
* set to one for TC's to use averages
*/
#define LL_TC_USE_AVG 0

#define LL_TC_AVG_NUM_READINGS 10

/**
* TC sensor read interval in millis.
*/
#define LL_TC_READ_INTERVAL 1000

/**
* Logging option flags:

#define PAD_CSV_SLAVE 1001 // flay to use for wifi slave, see https://github.com/freephases/wifi-plotly-slave
#define RAW_CSV 1002 // flag for using raw CSV data to serial 0, DEBUG_TO_SERIAL and DEBUG_SLAVE_SERIAL must be set to 0
#define NO_LOGGING 1000 //just use debugging mode, be sure to set DEBUG_TO_SERIAL to 1
*/

/**
* Maxium length of data reviced from slaves for 1 record/line
*/
#define MAX_STRING_DATA_LENGTH 130

/**
* Maxium length of data reviced from slaves for 1 record/line with less data, help keep mem use down if you use the right one
*/
#define MAX_STRING_DATA_LENGTH_SMALL 54

/**
* Default send interval when setting 'send_interval_sec' is missing from the 'RUN' file on SD card
*/
const unsigned long defaultSendDataIntervalSecs = 15;


#endif
