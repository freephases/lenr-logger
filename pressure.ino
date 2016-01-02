/**
* LENR logger pressure related monitoring functions
*/

// TODO:  calibration, get system voltage / 1023
int pressurePsi = 0;
int pressureRawV = 0;
const int pressurePort = A12;
const int readPressureInterval = 10000;
unsigned long readPressureMillis = 0;


void setupPressure()
{
  //pinMode(pressurePort, INPUT);
}

void readPressure()
{
  /*
  Pressure Range:  -14.5~30 PSI
  Input :5.0VDC (last mesured, using own varible VR's so can set voltage
  Output: 0.5-4.5V linear voltage output .
  */
  if (millis() - readPressureMillis >= readPressureInterval) {
    readPressureMillis = millis();
    pressureRawV = analogRead(pressurePort);
    pressurePsi = map(pressureRawV, 102, 921, -14, 30);
    if (debugToSerial) {
      Serial.print("Pressure raw: ");
      Serial.print(pressureRawV);
      Serial.print(", mapped PSI: ");
      Serial.println(pressurePsi);
    }
  }
}

int getPressurePsi()
{
  return pressurePsi;
}

