# lenr-logger
Arduino Based Logger


LENR logger
Jan 2015
Version: 0.0.1.6

Uses:
- Arduino Mega ATmega1280
_ SD card compatible with SD and SPI libs
- MAX31855 with thermocouple x 2 (at least, can have up to 4)
- Uno runnning with old style wifi card (rev1) - The wifi slave
  runs https://github.com/freephases/wifi-plotly-slave
- 5v transducer -14.5~30 PSI 0.5-4.5V linear voltage output
- Arduino Pro Mini with a OpenEnergyMonitor SMD card using analog ports 0-1 only - the power/emon slave
  runs https://github.com/freephases/power-serial-slave.git
- SSR to control heater power supply (see powerheater tab)
- Arduino Pro Mini with Lcd and keypad to display basic values and control PID/SSR run and stop
  runs https://github.com/freephases/lenr-logger-lcd.git
