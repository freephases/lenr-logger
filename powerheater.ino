/**
  Heater Power Control, controls SSR to heater power supply
  
  April 2016 - added overshoot, error corrention for PID controller response curve
  only using power off temp's now
  need to add watt or I limit to h-bridge driver
  http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/
  
*/
const unsigned long powerheaterCheckInterval = 1000;
unsigned long powerheaterMillis = 0;
OnOff heaterPowerControl(36);
OnOff heaterPowerControl2(37);
boolean heaterTimedOut = true; // true if reached maxRunTimeAllowedMillis
#define PID_MAX_PROGRAMS 5
float powerOverSteps[PID_MAX_PROGRAMS];
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
unsigned long dataStreamStartedMillis = 0;
boolean powerHeaterAutoMode = true;
unsigned long totalRunTimeMillis = 0;
unsigned long programsRunStartMillis = 0;
int lowestPowerPercentage = 79;//% can be set by SD card config settings lowest_power
int currentRunNum = 1;//used for display of currentProgram due to repating programs
//Define Variables we'll be connecting to for PID maths
double Setpoint, Input, Output=0;
// PID tuning
double KP=45;    // 2.2 degrees out = 100% heating
double KI=0.05;  // 3% per degree per minute
double KD=0;     // Not yet used

 
float firstError,lastError,currentError = 0.00;
unsigned long lastTurnedOffMillis = 0;
unsigned long lastTurnedOnMillis = 0;
float lastHighestTemp = 0.00;
boolean tempDesending = false;
int heaterMode = 0;
#define HEATERMODE_START 0
#define HEATERMODE_COLDSTART 1
#define HEATERMODE_CONSTANT 2
#define HEATERMODE_OFF -1

//Specify the links and initial tuning parameters
/*
where K_p, K_i, and K_d, all non-negative, denote the coefficients for the proportional, integral, and derivative terms, respectively (sometimes denoted P, I, and D). In this model,

P accounts for present values of the error. For example, if the error is large and positive, the control output will also be large and positive.
I accounts for past values of the error. For example, if the current output is not sufficiently strong, error will accumulate over time, and the controller will respond by applying a stronger action.
D accounts for possible future values of the error, based on its current rate of change.[1]
*/
//                                  ,Kp,Ki,Kd,
PID myPID(&Input, &Output, &Setpoint, KP,KI,KD, DIRECT);
double consKp=1, consKi=0.05, consKd=0.25;
void heaterOn()
{ 
  lastTurnedOnMillis = millis(); 
  lastTurnedOffMillis = 0;
  tempDesending = false;
  lastHighestTemp = 0;
  heaterPowerControl.on();
  heaterPowerControl2.on();

  if (controlHbridge) {
    hBridgeTurnOn();
  }
}

void heaterOff()
{
  lastTurnedOffMillis = millis();
  lastTurnedOnMillis = 0;
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
  
  lowestPowerPercentage = getConfigSettingAsInt("lowest_power", lowestPowerPercentage);

  repeatProgramsBy = getConfigSettingAsInt("repeat", 0);
  /**
    Load the programs to run
  */
  for (int i = 0; i < PID_MAX_PROGRAMS; i++) {

    if (getValue(getConfigSetting("power_off_temp") + ',', ',', i).length() > 0) {

      if (getValue(getConfigSetting("power_off_temp") + ',', ',', i).toFloat()==NAN) {
        if (debugToSerial) {
          Serial.print ("Error reading power_off_temp value: ");
          Serial.println(getValue(getConfigSetting("power_off_temp") + ',', ',', i));          
        }
        break; // bad value, stop processing power_on_temp settings
      }
      
      powerOverSteps[i] = 5.75;//10.00;//getValue(getConfigSetting("power_oversteps") + ',', ',', i).toFloat();
      powerOffTemps[i] = getValue(getConfigSetting("power_off_temp") + ',', ',', i).toFloat();
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
          Serial.print(" step: ");
          Serial.print(powerOverSteps[i], DEC);
          Serial.print(" time: ");
          Serial.println(maxRunTimeAllowedMillis[i]);
        }
    } else {
      break; // stop processing power/program settings
    }
  }
 
  Input = 0;//analogRead(0);
  Setpoint = powerOffTemps[0];
  //tmp hBridgeSpeed = 100
  //hBridgeSpeed = 100;
  myPID.SetOutputLimits(lowestPowerPercentage, hBridgeSpeed);
  myPID.SetSampleTime(powerheaterCheckInterval);
    
  powerheaterMillis = millis()-powerheaterCheckInterval;
}

void heaterPidStart()
{
      Input = getThermocoupleAvgCelsius1();
      Setpoint = powerOffTemps[currentProgram];
      firstError = powerOverSteps[currentProgram];
      if (abs(Setpoint-Input)<15.0) {
        firstError = firstError/2.0;
      }
      currentError = 0.00;
      heaterMode = HEATERMODE_START;
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
          heaterMode = HEATERMODE_OFF;
          heaterOff();
          lcdSlaveMessage('S', "ok");
          return;
        }
        
      } else {
        currentProgramMillis = 0;//reset current program timer
      }
      //Set PID vars
      heaterPidStart();
     
    }
  
}


int lastHBridgeSpeed = 0;

void manageRun()
{
  if (heaterTimedOut) {
    return;// not running
  }  
  
  Input = getThermocoupleAvgCelsius1(); 
  myPID.Compute();
  
  //if 20% near set point and temp going up, reduce power
  
  if (lastHBridgeSpeed != Output) {
        lastHBridgeSpeed = (int) Output;
        setHBridgeSpeed(lastHBridgeSpeed);
      }    
  
  manageHeater();
 
}

void manageHeater()
{
  //if (firstError>0.00&&Input>powerOffTemps[currentProgram]) firstError = 0.00;
  
  float minPoint = (powerOffTemps[currentProgram]-firstError)+currentError;//-(firstError>0.01?0.0:(powerOverSteps[currentProgram]/5.0));
 // float maxPoint = powerOffTemps[currentProgram]-1.50;
 // float powerTempOff = firstError>0.00?powerOffTemps[currentProgram]-firstError:powerOffTemps[currentProgram]-(powerOverSteps[currentProgram]/5.00);
//  float powerTempOn = powerOffTemps[currentProgram];
Serial.print("SPEED: ");
Serial.print(lastHBridgeSpeed);
 Serial.print("% - Temp: ");
    Serial.print(Input);
  Serial.print(", Threshold: ");
   Serial.print(minPoint);
//  Serial.print(" to ");
//    Serial.print(maxPoint);
  Serial.print(", Mode: ");
 Serial.println(heaterMode);
  
  switch(heaterMode) {
   case HEATERMODE_START:
       if (Input>= minPoint) {
         heaterMode = HEATERMODE_CONSTANT;
         currentError = 0.5;//powerOverSteps[currentProgram]/1.55;
         firstError = 0.00;        
       } else {
         heaterMode = HEATERMODE_COLDSTART;
         heaterOn();
       }
     break;
     
   case HEATERMODE_COLDSTART:
        if (firstError>0.01 && Input>=minPoint) {
          heaterOff();
         // heaterMode = HEATERMODE_CONSTANT;
          //firstError = 0.00;
          //myPID.SetTunings(consKp, consKi, consKd);
          firstError = 0.00;
          currentError = powerOverSteps[currentProgram]+powerOverSteps[currentProgram]/2.0;//for 200=5.75;
          
        } else if (firstError<0.01 && (Input>=minPoint||millis()-lastTurnedOffMillis>25000)) {
            currentError = powerOverSteps[currentProgram];
            //heaterOn();
            heaterMode = HEATERMODE_CONSTANT;                     
          }
        
     break;
     
   case HEATERMODE_CONSTANT:
   
                 
        if (heaterPowerControl.getIsOn() && (Input>minPoint || lastHBridgeSpeed==lowestPowerPercentage)) {
          heaterOff();                  
          currentError = 0;
          
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
          
        } else if (!heaterPowerControl.getIsOn() && (lastHBridgeSpeed>lowestPowerPercentage || Input<minPoint)) {
          heaterOn();         
        }
     break;
   default:
     break;
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
     
      runStartedMillis = millis();
      programsRunStartMillis = 0;
      currentProgramMillis = 0;
      currentRunNum = 1;
      currentProgram = 0;
      
      heaterPidStart();
      // currentProgramMillis = millis();
      lcdSlaveMessage('R', "ok");
      //turn the PID on
      myPID.SetMode(AUTOMATIC);
      heaterOn();
     
    } else { 
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
    //turn the PID on
    myPID.SetMode(MANUAL);
    heaterTimedOut = true;
    heaterMode = -1;
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
  return  repeatProgramsBy > 1 ? powerTempsCount*repeatProgramsBy : powerTempsCount; 
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


