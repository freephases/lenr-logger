/**
* LENR logger pressure related monitoring functions
*/


int pressurePsi = 0;
int pressureRawV = 0;
const int pressurePort = A12;
const int readPressureInterval = 500;
unsigned long readPressureMillis = 0;

int calFactor = 0;
void setupPressure()
{
  //pinMode(pressurePort, INPUT);
  calFactor = (int) 1023.00/calibratedVoltage;
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
    pressureRawV = analogRead(pressurePort);
    
    pressurePsi = map(pressureRawV, (int)calFactor*0.5, (int) calFactor*4.5, -14, 30);
    /*if (debugToSerial) {
      Serial.print("Pressure raw: ");
      Serial.print(pressureRawV);
      Serial.print(", mapped PSI: ");
      Serial.println(pressurePsi);
    }*/
  }
}

int getPressurePsi()
{
  return pressurePsi;
}

int getPressureRaw()
{
  return pressureRawV;
}
