//PLOTLY API SETTINGS ---------------------------------------------------------------------------------
//get from sd card - todo...
#define plotlyUserName "username"
#define plotlyPassword "password"
#define plotlyFilename "filename"
//tokens as pipe '|' delimited string eg: dgfhr647sgh|dhfjruskw8f
//each token is 10 chars

#define plotlyTokens "xxxxxxxxxxx|xxxxxxxxxxx|xxxxxxxxxxx|xxxxxxxxxxx|"
//you can have up to 6 with the wifi slave
#define plotlyTokenCount 4
//do we create a new chart or append data to exiting one...
int overWriteChart = 0;//should be a user set button option on device at run time
int plotlyMaxPoints = 300;//data is sent every 30 secs by deafault see: sendDataInterval in  main file


//Plotly tokens index for each trace
#define TRACE_CORE_TEMP   0
#define TRACE_ROOM_TEMP   1
#define TRACE_POWER       2
#define TRACE_PRESSURE    3

//PLOTLY API SETTINGS ---------------------------------------------------------------------------------

