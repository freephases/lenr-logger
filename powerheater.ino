/**
* Heater Power Control, controls SSR to heater power supply
*/
const unsigned long powerheaterCheckInterval = 505;
unsigned long powerheaterMillis = 0;
OnOff heaterPowerControl(36);
OnOff heaterPowerControl2(37);
boolean experimentJustStarted = true;
boolean heaterTimedOut = false; // true if reached maxRunTimeAllowedMillis
float powerOffTemp = 1000.1;
float powerOnTemp = 999.99;
#define PID_MAX_PROGRAMS 6
float powerOnTemps[PID_MAX_PROGRAMS];
float powerOffTemps[PID_MAX_PROGRAMS];
unsigned long maxRunTimeAllowedMillis[PID_MAX_PROGRAMS];
int powerTempsCount = 0;
int currentProgram = 0;
int repeatProgramsBy = 0;
int repeatProgramsByCount = 0;
unsigned long currentProgramMillis = 0;
unsigned long totalRunMillis = 0;
const unsigned long intervalBeforePower = 60000;
unsigned long dataStreamStartedMillis = 0;
boolean powerHeaterAutoMode = true;
unsigned long totalRunTimeMillis = 0;
unsigned long millisToFirstProgramTarget = 0;

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



void powerheaterSetup()
{
  heaterOff();
  //powerOnTemp = getConfigSettingAsFloat("power_on_temp", powerOnTemp);
  //powerOffTemp = getConfigSettingAsFloat("power_off_temp", powerOffTemp);
  repeatProgramsBy = getConfigSettingAsInt("repeat", 0);
  /**
  * Load the programs to run
  */
  //Serial.println(getConfigSetting("run_time_mins"));
  for(int i=0; i<PID_MAX_PROGRAMS; i++) {
    
    if (getValue(getConfigSetting("power_on_temp")+',', ',', i).length()>0) {
      
      powerOnTemps[i] = getValue(getConfigSetting("power_on_temp")+',', ',', i).toFloat();
     
      powerOffTemps[i] = getValue(getConfigSetting("power_off_temp")+',', ',', i).toFloat();
      if (getValue(getConfigSetting("run_time_mins")+',', ',', i).toInt()==0) maxRunTimeAllowedMillis[i] = 0;
      else maxRunTimeAllowedMillis[i] = (unsigned long) (60000 * getValue(getConfigSetting("run_time_mins")+',', ',', i).toInt());
      
      totalRunTimeMillis += maxRunTimeAllowedMillis[i];
      
     //  Serial.print(maxRunTimeAllowedMillis[i], DEC);Serial.println(" wwww ");
      powerTempsCount++;
    } else {
      break;
    }
  }
    
//  if (getConfigSettingAsInt("run_time_mins", 0) == 0) {
//    maxRunTimeAllowedMillis = 0;//
//  } else {
//    maxRunTimeAllowedMillis = (unsigned long) 60000 * getConfigSettingAsInt("run_time_mins");
//  }
  powerheaterMillis = millis();
}



void powerheaterLoop()
{
  if (powerHeaterAutoMode && millis() - powerheaterMillis >= powerheaterCheckInterval) {
    powerheaterMillis = millis();
    if (currentProgramMillis != 0 && millis() - currentProgramMillis >= maxRunTimeAllowedMillis[currentProgram])  {
      currentProgram++;
      if (currentProgram==powerTempsCount) {
        currentProgram = 0;//reset to first program
        repeatProgramsByCount++;//repeat count
        if (repeatProgramsBy>0 && repeatProgramsByCount<repeatProgramsBy) {
          repeatProgramsByCount++;
          currentProgramMillis = millis();
        } else {        
          repeatProgramsByCount = 0;
          heaterTimedOut = true;       
          heaterOff();
          lcdSlaveMessage('S', "ok");
          return;
        }
      } else {
        currentProgramMillis = millis();
      }
    }
    
    if (heaterPowerControl.getIsOn()) {

      //heater is currently on
      if (getThermocoupleAvgCelsius1() >= powerOffTemps[currentProgram]) {
        heaterOff();
        if (millisToFirstProgramTarget==0) 
        {
          millisToFirstProgramTarget = millis();
        }
        if (currentProgramMillis == 0) {
          if (maxRunTimeAllowedMillis[currentProgram] == 0) {
            heaterTimedOut = true;
          } else {
            //run time has now started we are at set temp
            currentProgramMillis = millis();
          }
        }
      }

    } else {
      // heater on...
      if (!experimentJustStarted && getThermocoupleAvgCelsius1() <= powerOnTemps[currentProgram]) {        
        heaterOn();       
      }
    }
  }

}

void startRun()
{
  if (!heaterPowerControl.getIsOn()) {
    setHBridgeSpeed(hBridgeSpeed);
    heaterTimedOut = false;
    if (powerHeaterAutoMode) {
      if (allowDataSend && !getTheStreamHasStarted()) {
        return;
      }
      heaterOn();
      totalRunMillis = millis(); 
      millisToFirstProgramTarget = 0;
      currentProgramMillis = 0;
     // currentProgramMillis = millis();
      experimentJustStarted = false;
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
    heaterTimedOut = true;
    currentProgramMillis = 0;
    repeatProgramsByCount = 0;
    experimentJustStarted = true;
    lcdSlaveMessage('S', "ok");
    totalRunMillis = 0;
  }
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
  return (millisToFirstProgramTarget != 0);
}

int getMinsToEndOfRun()
{
  unsigned long totalTimeLeft =0;
  unsigned long millisNow = millis();
 
 int repeatTotal = repeatProgramsBy>0?repeatProgramsBy:1;
 
 totalTimeLeft= totalRunTimeMillis * ((unsigned long) repeatTotal -(unsigned long)  repeatProgramsByCount);
 
 //totalTimeLeft += millisToFirstProgramTarget;
 
 // Serial.println(totalTimeLeft, DEC);
  if (hasRunStarted()) {
   if (currentProgramMillis>0) totalTimeLeft= totalTimeLeft-(millisNow-currentProgramMillis);  
   totalTimeLeft = (totalTimeLeft - (millisNow - millisToFirstProgramTarget)) / 60000;//((millis()-totalTimeLeft) - (millis()-currentProgramMillis)) / 60000;
 
    return (int) totalTimeLeft;
  } else {
    return (totalTimeLeft / 60000);
  }
}

int getTotalRunMins(int minsToEnd)
{
  if (hasRunStarted()) {    
    unsigned long totalTimeLeft = totalRunTimeMillis/60000;
    totalTimeLeft = totalTimeLeft-(unsigned long) minsToEnd;
    return (int) totalTimeLeft;
  } else {
    return 0;
  }
}

int getTotalRunTime()
{
  if (totalRunMillis!=0) {    
    unsigned long totalTime = totalRunMillis/60000;   
    return (int) totalTime;
  } else {
    return 0;
  }
}
