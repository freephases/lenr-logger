/**
* Power Slave, gets power values over serial from mini pro running 
* this: https://github.com/freephases/power-serial-slave.git
*/
short psPos = 0; // position in read buffer
char psBuffer[MAX_STRING_DATA_LENGTH + 1];
char psInByte = 0;
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
void processPowerSlaveResponse()
{
  char recordType = psBuffer[0];

  switch (recordType) {
    case 'R' : //got a response record/data from the our power slave
      setPowerInfo();
      break;

  }
}
/**
 * Process serial data sent to master from power slave
 */
void processPowerSlaveSerial()
{
  while (Serial1.available() > 0)
  {
    psInByte = Serial1.read();

    // add to our read buffer
    psBuffer[psPos] = psInByte;
    psPos++;

    if (psInByte == '\n' || psPos == MAX_STRING_DATA_LENGTH) //end of max field length
    {
      psBuffer[psPos - 1] = 0; // delimited
      processPowerSlaveResponse();
      psBuffer[0] = '\0';
      psPos = 0;
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
  return  emontx.Vrms * emontx.Irms;
}

void powerSlaveSetup() {
  Serial1.begin(9600);//power slaves serial
}

