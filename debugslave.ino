/**
* LENR logger slave debug interface
*/
char slaveInByte = 0;

void relaySerial2ToSerial()
{
  // send data only when you receive data:
    while (Serial2.available() > 0)
    {
      // read the incoming byte from slave and dump to our main serial
      slaveInByte = Serial2.read();
      Serial.print(slaveInByte);
    
    }
}
/**
 * Process data sent vy master
 */
void processDebugSlaveSerial()
{
  switch (DATA_LOGGERING_MODE) {
    case PAD_CSV_SLAVE: 
        relaySerial2ToSerial();
    break;
    default:
      Serial.println("processDebugSlaveSerialin debugslave is not complete");
  }
}


