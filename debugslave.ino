/**
* LENR logger slave debug interface
*/
#if DEBUG_SLAVE_SERIAL == 2
SoftwareSerial debugWifiSlave(12,34);

char slaveInByte = 0;//to hold in coming byte

void relaySerial2ToSerial()
{
  // send data only when you receive data:
      debugWifiSlave.listen();
    while (debugWifiSlave.available() > 0)
    {
      // read the incoming byte from slave and dump to our main serial
      slaveInByte = debugWifiSlave.read();      
      Serial.print(slaveInByte); //dump stright to serial 0 
    
    }
}
/**
 * Select debug print method, if any at all
 */
void processDebugSlaveSerial()
{
    if (allowDataSend) {
      relaySerial2ToSerial();
      
    }
}

void debugSetup()
{
  debugWifiSlave.begin(9600);
}
#else

void debugSetup()
{
}

void processDebugSlaveSerial()
{
}
#endif

