//START PLOTLY API SETTINGS ---------------------------------------------------------------------------------
//get from sd card - todo...
#define plotlyUserName "username"
#define plotlyPassword "password"
#define plotlyFilename "filename"
//token are 11 chars
//tokens pipe delimited eg: dgfhr647sgh|dhfjruskw8f| 
// don't forget leading pipe                       ^ - it's a bug
#define plotlyTokens "xxxxxxxxxxx|xxxxxxxxxxx|xxxxxxxxxxx|xxxxxxxxxxx|"
#define plotlyTokenCount 4
int overWriteChart = 1;//should be a user set button option on device at run time
int plotlyMaxPoints = 130;//data is sent every 30 secs by deafault see: sendDataInterval in  main file
//END PLOTLY API SETTINGS ---------------------------------------------------------------------------------

