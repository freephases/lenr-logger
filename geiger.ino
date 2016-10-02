#if (SERIAL1_USAGE == S1_GC10)

int geigerCpm = 0;
/**
* LENR logger Geiger counter related functions
* GC-10 using it's tx port only
*/
int geigerGetCpm() 
{
  return geigerCpm;
}

/**
* Do something with response from geiger counter
*/
void processSerial1Response()
{
  String s(serial1Buffer);
  geigerCpm = s.toInt();
}

#endif
