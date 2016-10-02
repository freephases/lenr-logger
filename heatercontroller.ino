/**
* Heater Control, controls SSR or/and Hbridge to heater power supply
* using PID lib that uses up to 5 specific run programs taken from
* "power_off_temp" config file setting (see: config tab/file)
*/
const unsigned long powerheaterCheckInterval = 1000;
unsigned long powerheaterMillis = 0;
OnOff heaterPowerControl(36);
OnOff heaterPowerControl2(37);
boolean heaterTimedOut = true; // true if reached maxRunTimeAllowedMillis
#define PID_MAX_PROGRAMS 5
float powerOffTemps[PID_MAX_PROGRAMS];
//float powerTempsError[PID_MAX_PROGRAMS];
unsigned long maxRunTimeAllowedMillis[PID_MAX_PROGRAMS];
int powerTempsCount = 0;
int currentProgram = 0;
int repeatProgramsBy = 0;
int repeatProgramsByCount = 0;
unsigned long currentProgramMillis = 0;
unsigned long runStartedMillis = 0;
const unsigned long intervalBeforePower = 60000;
//unsigned long dataStreamStartedMillis = 0;
boolean powerHeaterAutoMode = true;
unsigned long totalRunTimeMillis = 0;
unsigned long programsRunStartMillis = 0;
int currentRunNum = 1;//used for display of currentProgram due to repating programs
//Define Variables we'll be connecting to for PID maths
double Setpoint, Input, Output;
unsigned long lastTurnedOffMillis = 0;
unsigned long lastTurnedOnMillis = 0;
boolean isManualRun = false;
const unsigned long WindowSize = 4000;
unsigned long windowStartTime = 0;


/*
where K_p, K_i, and K_d, all non-negative, denote the coefficients for the proportional, integral, and derivative terms, respectively (sometimes denoted P, I, and D). In this model,

P accounts for present values of the error. For example, if the error is large and positive, the control output will also be large and positive.
I accounts for past values of the error. For example, if the current output is not sufficiently strong, error will accumulate over time, and the controller will respond by applying a stronger action.
D accounts for possible future values of the error, based on its current rate of change.[1]
*/
// PID tuning
double KP = 99;  // 1 degree error will increase the power by KP percentage points
double KI = 0.01; // KI percentage points per degree error per second,
double KD = 0;   // Not yet used
//                                  ,Kp,Ki,Kd,
PID myPID(&Input, &Output, &Setpoint, KP, KI, KD, DIRECT);
//double consKp=1, consKi=0.05, consKd=0.25;

void heaterOn()
{
  if (!heaterPowerControl.getIsOn()) {
    lastTurnedOnMillis = millis();
    lastTurnedOffMillis = 0;

    heaterPowerControl.on();
    heaterPowerControl2.on();

    if (controlHbridge) {
      hBridgeTurnOn();
    }
    if (debugToSerial) {
      Serial.println("Heater on");
    }
  }
}

/**
* heaterOff
*
* @param doNotCheckOnStatus boolean - if true do not check if heater is on before turning off, default is to check (false)
*/
void heaterOff(boolean doNotCheckOnStatus = false)
{
  if (doNotCheckOnStatus || heaterPowerControl.getIsOn()) {
    lastTurnedOffMillis = millis();
    lastTurnedOnMillis = 0;
    heaterPowerControl.off();
    heaterPowerControl2.off();
    if (controlHbridge) {
      hBridgeTurnOff();
    }
    if (debugToSerial) {
      Serial.println("Heater off");
    }
  }
}

/**
* Load power/PID settings
*/
void powerheaterSetup()
{
  heaterOff(true);//makes sure it's off incase of reboot without a turn off heater command before hand

  repeatProgramsBy = getConfigSettingAsInt("repeat", 0);
  /**
    Load the programs to run
  */
  for (int i = 0; i < PID_MAX_PROGRAMS; i++) {

    if (getValue(getConfigSetting("power_off_temp") + ',', ',', i).length() > 0) {

      if (getValue(getConfigSetting("power_off_temp") + ',', ',', i).toFloat() == NAN) {
        if (debugToSerial) {
          Serial.print ("Error reading power_off_temp value: ");
          Serial.println(getValue(getConfigSetting("power_off_temp") + ',', ',', i));
        }
        break; // bad value, stop processing power_on_temp settings
      }

      //powerOverSteps[i] = getValue(getConfigSetting("power_oversteps") + ',', ',', i).toFloat();
      powerOffTemps[i] = getValue(getConfigSetting("power_off_temp") + ',', ',', i).toFloat();
      if (powerOffTemps[i] > manualMaxTemp) {
        powerOffTemps[i] = manualMaxTemp;
      }
      //powerTempsError[i] = 0.00;//set to zero to start with

      if (getValue(getConfigSetting("run_time_mins") + ',', ',', i).toInt() == 0) maxRunTimeAllowedMillis[i] = 0;
      else maxRunTimeAllowedMillis[i] = (unsigned long) (60000 * (unsigned long) getValue(getConfigSetting("run_time_mins") + ',', ',', i).toInt());

      totalRunTimeMillis += maxRunTimeAllowedMillis[i];
      powerTempsCount++;

      if (debugToSerial) {
        Serial.println("Power temp ");
        Serial.print(powerTempsCount);
        //Serial.print(" added - on: ");
        //Serial.print(powerOnTemps[i], DEC);
        Serial.print(" off: ");
        Serial.print(powerOffTemps[i], DEC);
        //Serial.print(" step: ");
        // Serial.print(powerOverSteps[i], DEC);
        Serial.print(" time: ");
        Serial.println(maxRunTimeAllowedMillis[i]);
      }
    } else {
      break; // stop processing power/program settings
    }
  }

  Input = 0;
  Setpoint = powerOffTemps[0];
  myPID.SetOutputLimits(0, WindowSize);
 
  myPID.SetSampleTime(powerheaterCheckInterval);//200mS is default
  powerheaterMillis = millis() - powerheaterCheckInterval;
}

/**
* `Set PID to use specific PID program
*/
void heaterProgramStart()
{
  Input = getControlTcTemp();
  Setpoint = powerOffTemps[currentProgram];
}

/**
* Check and switch program to run or end run
*/
void checkProgramToRun()
{
  if (currentProgramMillis != 0 && millis() - currentProgramMillis >= maxRunTimeAllowedMillis[currentProgram])  {
    currentProgram++;
    currentRunNum++;
    if (currentProgram == powerTempsCount) {
      currentProgram = 0;//reset to first program
      repeatProgramsByCount++;//repeat count
      if (repeatProgramsBy > 0 && repeatProgramsByCount < repeatProgramsBy) {
        repeatProgramsByCount++;
        currentProgramMillis = 0;//reset current program timer
      } else {
        currentRunNum = 0;
        repeatProgramsByCount = 0;
        heaterTimedOut = true;
        heaterOff();
        lcdSlaveMessage('S', "ok");
        return;
      }
    } else {
      currentProgramMillis = 0;//reset current program timer
    }
    //Set specific PID vars for this program
    heaterProgramStart();
  }
}

/**
* set timer watchers
*/
void startProgramTimeTrackers()
{
  if (programsRunStartMillis == 0)
  {
    programsRunStartMillis = millis();
  }
  if (currentProgramMillis == 0) {
    if (maxRunTimeAllowedMillis[currentProgram] == 0) {
      heaterTimedOut = true;//0 means user now controls on off tbc todooo
    } else {
      //count down gas started now we are at set temp for current program
      currentProgramMillis = millis();
    }
  }
}

/**
* Check temp if in manual run
*/
void monitorManualRun() {
  if (heaterPowerControl.getIsOn()) {
    if (getControlTcTemp() > manualMaxTemp) {
      heaterOff();
    }
  } else if (isManualRun && getControlTcTemp() < manualMaxTemp) {
    heaterOn();
  }
}

void powerDebug() {
  if (debugToSerial) {
    Serial.print("setpoint: "); Serial.print(Setpoint); Serial.print(" ");
    Serial.print("input: "); Serial.print(Input); Serial.print(" ");
    Serial.print("output: "); Serial.print(Output); Serial.print(" ");
    //
    Serial.print("kp: "); Serial.print(myPID.GetKp()); Serial.print(" ");
    Serial.print("ki: "); Serial.print(myPID.GetKi()); Serial.print(" ");
    Serial.print("kd: "); Serial.print(myPID.GetKd()); Serial.println();
  }
}

/**
* Turn heater on or off for this specific PID program
*/
void manageRun()
{
  if (heaterTimedOut) {
    monitorManualRun();
    return;// not running
  }

  Input = getControlTcTemp();
  myPID.Compute();

  if (millis() - windowStartTime > WindowSize)
  {
    windowStartTime += WindowSize;
  }

  if (Input >= Setpoint) {
    //reached target temp now set timers for current program
    startProgramTimeTrackers();
  }
  //Serial.println(millis() - windowStartTime);
  if (Output < millis() - windowStartTime) {
    heaterOff();
  }
  else {
    heaterOn();
  }
  
  powerDebug();
}


void powerheaterLoop()
{
  if (!heaterTimedOut && powerHeaterAutoMode && millis() - powerheaterMillis >= powerheaterCheckInterval) {
    powerheaterMillis = millis();
    checkProgramToRun();
    manageRun();
  }
}

/**
* Set the run of all programs or run in manual - always on mode
*/
void startRun()
{
  if (!heaterPowerControl.getIsOn()) {
    setHBridgeSpeed(hBridgeSpeed);//reset to config setting in case user has been messing around with values
    heaterTimedOut = false;
    if (powerHeaterAutoMode) {
      windowStartTime = millis();
      runStartedMillis = millis();
      programsRunStartMillis = 0;
      currentProgramMillis = 0;
      currentRunNum = 1;
      currentProgram = 0;
      isManualRun = false;
      heaterProgramStart();
      // currentProgramMillis = millis();
      lcdSlaveMessage('R', "ok");
      //turn the PID on
      myPID.SetMode(AUTOMATIC);
//      heaterOn();
    } else {
      //manual mode
      heaterOn();
      heaterTimedOut = true;//no timmer
      isManualRun = true;
      lcdSlaveMessage('R', "ok");
    }
  }
}

void stopRun()
{
  if (heaterPowerControl.getIsOn()) {
    heaterOff();
  }
  //turn the PID on
  myPID.SetMode(MANUAL);
  heaterTimedOut = true;
  isManualRun = false;
  //heaterMode = -1;
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

int getTotalProgramsToRun()
{
  return  repeatProgramsBy > 1 ? powerTempsCount * repeatProgramsBy : powerTempsCount;
}

int getCurrentProgramNum()
{
  return currentRunNum;
}

int getMinsToEndOfRun()
{
  unsigned long totalTimeLeft = 0;
  unsigned long millisNow = millis();

  int repeatTotal = repeatProgramsBy > 1 ? repeatProgramsBy : 1;
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


