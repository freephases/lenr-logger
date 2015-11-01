/**
* LENR logger slave debug interface
*/
#if DEBUG_SLAVE_SERIAL == 1

char slaveInByte = 0;//to hold in coming byte

void relaySerial2ToSerial()
{
  // send data only when you receive data:
    while (Serial2.available() > 0)
    {
      // read the incoming byte from slave and dump to our main serial
      slaveInByte = Serial2.read();      
      Serial.print(slaveInByte); //dump stright to serial 0 
    
    }
}
/**
 * Select debug print method, if any at all
 */
void processDebugSlaveSerial()
{
    switch (DATA_LOGGERING_MODE) {
      case PAD_CSV_SLAVE: 
          relaySerial2ToSerial();
      break;
      // ..
    }
}

#endif

