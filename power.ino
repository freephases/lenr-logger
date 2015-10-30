/**
* LENR logger power related monitoring functions
*/

EnergyMonitor ACsensor; 
const long readPowerInterval = 2500;//read every 2.5 sec, calcVI times out at 2 secs max
unsigned long readPowerMillis = 0; // last milli secs since last avg reading
typedef struct { float power, Vrms, Irms;} PayloadTX;      // create structure - a neat way of packaging data for RF comms
PayloadTX emontx;  

void setupPower()
{
  ACsensor.current(1, 155.3);//60.606);    //111.1
  ACsensor.voltage(0, 268.97, 1.7);    
}


void readPower() {
   if (millis()-readPowerMillis>=readPowerInterval) {
    readPowerMillis = millis();
   ACsensor.calcVI(20, 2000);  // Calculate 
   emontx.power = ACsensor.realPower;
   emontx.Vrms = ACsensor.Vrms;//*100;    
   emontx.Irms = ACsensor.Irms;    
   
  if (DEBUG_TO_SERIAL==1) {
    
     // VI = ACsensor.Vrms;
      Serial.print(" P: ");
      Serial.print(getPower());	
      Serial.print(" AP: ");
      Serial.print(getApparentPower());	
      Serial.print(" V: ");
      Serial.print(getVrms());	
      Serial.print(" I: ");
      Serial.println(getIrms());	
    // ACsensor.serialprint(); 
   }
  }
}

float getPower() {
  return emontx.power;
}
float getVrms() {
  return  emontx.Vrms;
}
float getIrms() {
  return emontx.Irms;
}

float getApparentPower() {
   return  emontx.Vrms*emontx.Irms;
}

