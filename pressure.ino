/**
* LENR logger pressure related monitoring functions
*/


float pressurePsi = 0.000;
float pressurePsiTotal = 0.000;
int pressureReadCounter = 0;
const int pressureReadCount = 4;
unsigned long pressureRawV = 0;
const int pressurePort = A12;
const int readPressureInterval = 250;
unsigned long readPressureMillis = 0;

float calFactor = 0.00;
void setupPressure()
{
  //pinMode(pressurePort, INPUT);
  calFactor =  1023.00/calibratedVoltage;
}
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void readPressure()
{
  /*
  Pressure Range:  -14.5~30 PSI
  Input :4.980 (last mesured, using own varible VR's so can set voltage
  Output: 0.5-4.5V linear voltage output .
  4.980 is mega when using wall socket
  */
  if (millis() - readPressureMillis >= readPressureInterval) {
    readPressureMillis = millis();
    pressureRawV += analogRead(pressurePort);
    //pressurePsi = mapfloat(pressureRawV, (int) calFactor*0.5, (int) calFactor*4.5, -14, 30);
    //pressurePsiTotal += mapfloat((float)pressureRawV, calFactor*0.5, calFactor*4.5, -14.5, 30.0);
    pressureReadCounter++;
    if (pressureReadCounter==pressureReadCount) {
      int p  = pressureRawV/pressureReadCount;
      pressurePsi = mapfloat((float)p, calFactor*0.5, calFactor*4.5, -14.5, 30.0);
    
      pressureReadCounter = 0;
      pressureRawV = 0;
    if (debugToSerial) {
     // Serial.print("Pressure raw: ");
     // Serial.print(pressureRawV);
      Serial.print(", mapped PSI: ");
      Serial.println(pressurePsi, DEC);
    }
    }
  }
}

float getPressurePsi()
{
  return pressurePsi;
}

int getPressureRaw()
{
  return pressureRawV;
}
