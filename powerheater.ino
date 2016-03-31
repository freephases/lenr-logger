/**
  Heater Power Control, controls SSR to heater power supply
*/
const unsigned long powerheaterCheckInterval = 505;
unsigned long powerheaterMillis = 0;
OnOff heaterPowerControl(36);
OnOff heaterPowerControl2(37);
boolean heaterTimedOut = true; // true if reached maxRunTimeAllowedMillis
#define PID_MAX_PROGRAMS 6
float powerOnTemps[PID_MAX_PROGRAMS];
float powerOffTemps[PID_MAX_PROGRAMS];
unsigned long maxRunTimeAllowedMillis[PID_MAX_PROGRAMS];
int powerTempsCount = 0;
int currentProgram = 0;
int repeatProgramsBy = 0;
int repeatProgramsByCount = 0;
unsigned long currentProgramMillis = 0;
unsigned long runStartedMillis = 0;
const unsigned long intervalBeforePower = 60000;
unsigned long dataStreamStartedMillis = 0;
boolean powerHeaterAutoMode = true;
unsigned long totalRunTimeMillis = 0;
unsigned long programsRunStartMillis = 0;

void heaterOn()
{  
  heaterPowerControl.on();
  heaterPowerControl2.on();

  if (controlHbridge) {
    hBridgeTurnOn();
  }
}
void heaterOff()
{
  heaterPowerControl.off();
  heaterPowerControl2.off();
  if (controlHbridge) {
    hBridgeTurnOff();
  }
}


/**
* Load power/PID settings
*/
void powerheaterSetup()
{
  heaterOff();

  repeatProgramsBy = getConfigSettingAsInt("repeat", 0);
  /**
    Load the programs to run
  */
  for (int i = 0; i < PID_MAX_PROGRAMS; i++) {

    if (getValue(getConfigSetting("power_on_temp") + ',', ',', i).length() > 0) {

      if (getValue(getConfigSetting("power_on_temp") + ',', ',', i).toFloat()==NAN) {
        if (debugToSerial) {
          Serial.print ("Error reading power_on_temp value: ");
          Serial.println(getValue(getConfigSetting("power_on_temp") + ',', ',', i));          
        }
        break; // bad value, stop processing power_on_temp settings
      }
      
      powerOnTemps[i] = getValue(getConfigSetting("power_on_temp") + ',', ',', i).toFloat();
      powerOffTemps[i] = getValue(getConfigSetting("power_off_temp") + ',', ',', i).toFloat();
      
      if (getValue(getConfigSetting("run_time_mins") + ',', ',', i).toInt() == 0) maxRunTimeAllowedMillis[i] = 0;
      else maxRunTimeAllowedMillis[i] = (unsigned long) (60000 * (unsigned long) getValue(getConfigSetting("run_time_mins") + ',', ',', i).toInt());
            
      totalRunTimeMillis += maxRunTimeAllowedMillis[i];
      powerTempsCount++;

      if (debugToSerial) {
          Serial.println("Power temp ");
          Serial.print(powerTempsCount);
          Serial.print(" added - on: ");
          Serial.print(powerOnTemps[i], DEC);
          Serial.print(" off: ");
          Serial.print(powerOffTemps[i], DEC);
          Serial.print(" time: ");
          Serial.println(maxRunTimeAllowedMillis[i]);
        }
    } else {
      break; // stop processing power/program settings
    }
  }
 
  powerheaterMillis = millis();
}


/**
* Check and switch program to run or end run
*/
void checkProgramToRun()
{
    if (currentProgramMillis != 0 && millis() - currentProgramMillis >= maxRunTimeAllowedMillis[currentProgram])  {
      currentProgram++;
      if (currentProgram == powerTempsCount) {
        currentProgram = 0;//reset to first program
        repeatProgramsByCount++;//repeat count
        if (repeatProgramsBy > 0 && repeatProgramsByCount < repeatProgramsBy) {
          repeatProgramsByCount++;
          
          currentProgramMillis = 0;//reset current program timer
        } else {
          repeatProgramsByCount = 0;
          heaterTimedOut = true;
          heaterOff();
          lcdSlaveMessage('S', "ok");
          return;
        }
      } else {
        currentProgramMillis = 0;//reset current program timer
      }
    }
  
}
int getTotalProgramsToRun()
{
  return powerTempsCount*repeatProgramsBy;
}


int getCurrentProgramNum()
{
  return (repeatProgramsByCount==0?0:repeatProgramsByCount*powerTempsCount)+currentProgram+1;
}


/**
* Check temp and turn on/off heater
*/
void manageRun()
{
  if (heaterTimedOut) {
    return;// not running
  }  
  
    //heater is currently on
    if (getThermocoupleAvgCelsius1() >= powerOffTemps[currentProgram]) {
          
      
        if (heaterPowerControl.getIsOn()) {
          heaterOff();
        }
        if (programsRunStartMillis == 0)
        {
          programsRunStartMillis = millis();
        }
        if (currentProgramMillis == 0) {
          if (maxRunTimeAllowedMillis[currentProgram] == 0) {
            heaterTimedOut = true;//0 means infinite
          } else {
            //count down gas started now we are at set temp for current program
            currentProgramMillis = millis();
          }
        }
      
      }
    
   else if (!heaterPowerControl.getIsOn() && getThermocoupleAvgCelsius1() <= powerOnTemps[currentProgram]) {
      heaterOn();
  }
}

void powerheaterLoop()
{
  if (!heaterTimedOut && powerHeaterAutoMode && millis() - powerheaterMillis >= powerheaterCheckInterval) {
    powerheaterMillis = millis();
    checkProgramToRun();
    manageRun();    
  }
}

void startRun()
{
  if (!heaterPowerControl.getIsOn()) {
    setHBridgeSpeed(hBridgeSpeed);
    heaterTimedOut = false;
    if (powerHeaterAutoMode) {
      //if (allowDataSend && !getTheStreamHasStarted()) {
      //  return;
      //}
      heaterOn();
      runStartedMillis = millis();
      programsRunStartMillis = 0;
      currentProgramMillis = 0;
      // currentProgramMillis = millis();
      lcdSlaveMessage('R', "ok");

    } else { //!allowDataSend) {
      //manual mode
      heaterOn();
      heaterTimedOut = true;//no timmer
      lcdSlaveMessage('R', "ok");
    }
  }
}

void stopRun()
{
  if (heaterPowerControl.getIsOn()) {
    heaterOff();
  }
    heaterTimedOut = true;
    currentProgramMillis = 0;
    repeatProgramsByCount = 0;
    lcdSlaveMessage('S', "ok");
    runStartedMillis = 0;
    programsRunStartMillis = 0;  
}

void setPowerHeaterAutoMode(boolean autoMode)
{
  powerHeaterAutoMode = autoMode;
  if (autoMode)
    lcdSlaveMessage('A', "ok"); // auto set ok
  else
    lcdSlaveMessage('a', "ok"); // manual set ok
}

boolean getPowerHeaterAutoMode()
{
  return powerHeaterAutoMode;
}

boolean hasRunStarted()
{
  return (programsRunStartMillis != 0);
}

int getMinsToEndOfRun()
{
  unsigned long totalTimeLeft = 0;
  unsigned long millisNow = millis();

  int repeatTotal = repeatProgramsBy > 0 ? repeatProgramsBy : 1;
  unsigned long totalRunTime = programsRunStartMillis > 0 ? millisNow - programsRunStartMillis : 0;
  totalTimeLeft = (totalRunTimeMillis * (unsigned long) repeatTotal) - totalRunTime;

  return (int) (totalTimeLeft / 60000);
}

unsigned long getTotalRunningTimeMillis()
{
  return runStartedMillis > 0 ? millis() - runStartedMillis : 0;
}

int getTotalRunningTimeMins()
{
  return runStartedMillis > 0 ? getTotalRunningTimeMillis() / 60000 : 0;
}


