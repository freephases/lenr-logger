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
    maxRunTimeAllowedMillis = (unsigned long) 60 * 1000 * getConfigSettingAsInt("run_time_mins");
  }
  powerheaterMillis = millis();
}



void powerheaterLoop()
{


  if (!heaterTimedOut && millis() - powerheaterMillis >= powerheaterCheckInterval) {
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

      //heater is currently off
      if (experimentJustStarted) {
        #if DATA_LOGGERING_MODE == PAD_CSV_SLAVE
          if (dataStreamStartedMillis==0) { 
            if (getTheStreamHasStarted()) {
              dataStreamStartedMillis = millis();
            }
          } else {
            if (millis()-dataStreamStartedMillis>intervalBeforePower) {
              heaterOn();
              experimentJustStarted = false;
            }
          }         
        #else
          if ( millis() > intervalBeforePower) { //auto turn on after intervalBeforePower millis
            experimentJustStarted = false;
            heaterOn();
          }
        #endif     
        
      }
      else if (getThermocoupleAvgCelsius1() <= powerOnTemp) {        
          heaterOn();        
      }

    }
  }

}
