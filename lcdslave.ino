/**
* Lcd slave - displays basic info in lcd and heater run controls from keypad
* see: https://github.com/freephases/lenr-logger-lcd.git
*/
//SoftwareSerial lcd(34,11);//using serial 2 now
short lcdPos = 0; // position in read buffer
char lcdBuffer[MAX_STRING_DATA_LENGTH + 1];
char lcdSlaveMsg[MAX_STRING_DATA_LENGTH + 1];
char lcdInByte = 0;
unsigned long lastLcdSlaveSendDataMillis = 0;

void lcdSlaveMessage(char c, String msg)
{
  char tmpMsg[80];
  char timeData[30];
  int timeLeft = 0;
  int totalTime = 0;

  msg.toCharArray(tmpMsg, 80);

  if (c == 'D' && getPowerHeaterAutoMode()) {

    timeLeft = getMinsToEndOfRun();
    totalTime = getTotalRunMins();
    sprintf(timeData, "%d|%d", timeLeft, totalTime);
    sprintf(lcdSlaveMsg, "%c|%s|%s|!", c, tmpMsg, timeData);
  } else {
    sprintf(lcdSlaveMsg, "%c|%s|!", c, tmpMsg);
  }

  // Serial2.flush();
  Serial2.println(lcdSlaveMsg);
  //  Serial.flush();

}


/*void lcdSlaveError(String msg)
{
  char tmpMsg[17];
  msg.toCharArray(tmpMsg, 17);
  sprintf(lcdSlaveMsg, "E|%s|!", tmpMsg);
  Serial2.println(lcdSlaveMsg);
}*/

void lcdSlaveSendData()
{
  if (millis() - lastLcdSlaveSendDataMillis > 1075) {
    lastLcdSlaveSendDataMillis = millis();
    char data[70];
    getCsvString('|', false).toCharArray(data, 70);
    lcdSlaveMessage('D', data);
    // sprintf(data, "D|%s|!", data);
    //  Serial2.println(data);
  }
}

/**
* Execute response
*/
void processLcdSlaveResponse()
{
  char recordType = lcdBuffer[0];
  if (debugToSerial) {
    Serial.print("LCD cmd: ");
    Serial.println(recordType);
  }

  switch (recordType) {
    case 'S' : //stop pid
      stopRun();
      break;
    case 'R' : //run pid
      startRun();
      break;
    case 'A' : //set to auto run mode
      setPowerHeaterAutoMode(true);
      break;
    case 'M' : //set manual mode
      setPowerHeaterAutoMode(false);
      break;
  }
}

void processLcdSlaveSerial()
{
  //Serial2.listen();
  while (Serial2.available() > 0)
  {
    lcdInByte = Serial2.read();
    // add to our read buffer
    lcdBuffer[lcdPos] = lcdInByte;
    lcdPos++;

    if (lcdInByte == '\n' || lcdPos == MAX_STRING_DATA_LENGTH) //end of max field length
    {
      lcdBuffer[lcdPos - 1] = 0; // delimited
      if (debugToSerial) {
        Serial.print("** LCD Response: ");
        Serial.println(lcdBuffer);
      }
      processLcdSlaveResponse();
      lcdBuffer[0] = '\0';
      lcdPos = 0;

    }
  }
}

void setupLcdSlave()
{
  Serial2.begin(9600);
  Serial2.println("****");
}
