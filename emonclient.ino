#if (SERIAL1_USAGE == S1_EMON)
/**
* Power Slave, gets power values over serial from mini pro running 
* this: https://github.com/freephases/power-serial-slave.git
* for AC measuring
*/

typedef struct {
  float power, Vrms, Irms;
} PayloadTX;     
PayloadTX emontx;


/**
* Set emontx struct
*/
void setPowerInfo() {
  char buf[15];
  float f;

  getValue(psBuffer, '|', 1).toCharArray(buf, 15);
  f = atof(buf);
  emontx.power = f;

  getValue(psBuffer, '|', 2).toCharArray(buf, 15);
  f = atof(buf);
  emontx.Vrms = f;

  getValue(psBuffer, '|', 3).toCharArray(buf, 15);
  f = atof(buf);
  emontx.Irms = f;
}

/**
* Execute response
*/
void processSerial1Response()
{
  char recordType = psBuffer[0];

  switch (recordType) {
    case 'R' : //got a response record/data from the our power slave
      setPowerInfo();
      break;

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
  return  emontx.Vrms * emontx.Irms;
}



#endif
