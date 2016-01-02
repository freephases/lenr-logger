/**
* Heater Power Control, controls SSR to heater power supply
*/

const unsigned long powerheaterCheckInterval = 1555;
unsigned long powerheaterMillis = 0;
OnOff heaterPowerControl(36);
OnOff heaterPowerControl2(37);
boolean experimentJustStarted = true;
boolean heaterTimedOut = false; // true if reached maxRunTimeAllowedMillis
float powerOffTemp = 1000.1;
float powerOnTemp = 999.99;
unsigned long maxRunTimeAllowedMillis = 0;
unsigned long runTimeMillis = 0;
const unsigned long intervalBeforePower = 60000;
unsigned long dataStreamStartedMillis = 0;
boolean powerHeaterAutoMode = true;

void heaterOn()
{
  heaterPowerControl.on();
  heaterPowerControl2.on();
}
void heaterOff()
{
  heaterPowerControl.off();
  heaterPowerControl2.off();
}

void powerheaterSetup()
{
  heaterOff();  
  powerOnTemp = getConfigSettingAsFloat("power_on_temp", powerOnTemp);
  powerOffTemp = getConfigSettingAsFloat("power_off_temp", powerOffTemp);
  if (getConfigSettingAsInt("run_time_mins", 0) == 0) {
    maxRunTimeAllowedMillis = 0;//
  } else {
    maxRunTimeAllowedMillis = (unsigned long) 60000 * getConfigSettingAsInt("run_time_mins");
  }
  powerheaterMillis = millis();
}



void powerheaterLoop()
{

  if (powerHeaterAutoMode && millis() - powerheaterMillis >= powerheaterCheckInterval) {
    powerheaterMillis = millis();
    
    if (runTimeMillis != 0 && millis() - runTimeMillis >= maxRunTimeAllowedMillis)  {
      heaterTimedOut = true;
      heaterOff();
      return;
    }

    if (heaterPowerControl.getIsOn()) {

      //heater is currently on
      if (getThermocoupleAvgCelsius1() >= powerOffTemp) {
        heaterOff();

        if (runTimeMillis == 0) {
          if (maxRunTimeAllowedMillis == 0) {
            heaterTimedOut = true;
          } else {       
            //run has started
            runTimeMillis = millis();
          }
        }
      }

    } else {
      // heater off...    
      if (!experimentJustStarted && getThermocoupleAvgCelsius1() <= powerOnTemp) {        
          heaterOn();        
      }
    }
  }

}

void startRun()
{
  if (!heaterPowerControl.getIsOn()) {
    heaterTimedOut = false;
    if(powerHeaterAutoMode) {
            if (allowDataSend && !getTheStreamHasStarted()) return;
              heaterOn();
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
    runTimeMillis = 0;
    experimentJustStarted = true;
    lcdSlaveMessage('S', "ok");
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
  return (runTimeMillis!=0);
}

int getMinsToEndOfRun()
{
  
  if (hasRunStarted()) {
    return (int) (maxRunTimeAllowedMillis/60000) - (millis()-runTimeMillis/60000);
  } else {
     return (int) (maxRunTimeAllowedMillis/60000);
  }
}

int getTotalRunMins()
{
  if (hasRunStarted()) {
   return (int) (millis()-runTimeMillis)/60000;
  } else {
    return 0;
  }
}
