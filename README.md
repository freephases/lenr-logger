# lenr-logger
Arduino Based Logger


LENR logger
Oct 2015
Version: 0.0.1.0

Uses:
- Arduino Mega ATmega1280
- max6675 with thermocouple x 2
- Uno with old style wifi card (rev1) - The wifi slave
  running https://github.com/freephases/wifi-plotly-slave
- 5v transducer -14.5~30 PSI 0.5-4.5V linear voltage output
- Arduino Pro Mini running OpenEnergyMonitor SMD card using analog ports 0-1 only - the power slave
  running https://github.com/freephases/power-serial-slave.git
