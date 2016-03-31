# lenr-logger
Arduino Based Logger


LENR logger
Mar 2016
Version: 0.0.1.8

Uses:
- Arduino Mega ATmega1280
_ SD card compatible with SD and SPI libs
- MAX31855 with thermocouple x 2 (at least, can have up to 4)
- Uno runnning with old style wifi card (rev1) - The wifi slave
  runs https://github.com/freephases/wifi-plotly-slave
- 5v transducer -14.5~30 PSI 0.5-4.5V linear voltage output
- SSR to control heater power supply (see powerheater tab)
- Arduino Pro Mini with Lcd and keypad to display basic values and control PID/SSR run and stop
  runs https://github.com/freephases/lenr-logger-lcd.git
- added h-bridge control via another MC as alturnitive or can be as at same time as SSR control
- added thermocouple linearization for maxim-31855 module
- added GC-10 Geiger counter

Optional:
- Arduino Pro Mini with a OpenEnergyMonitor SMD card using analog ports 0-1 only - the power/emon slave
  runs https://github.com/freephases/power-serial-slave.git