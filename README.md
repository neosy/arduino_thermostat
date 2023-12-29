# arduino_thermostat
Arduino-based thermostat.
The thermostat was designed to control an Arduino-based water heater.

## Installation
Copy folders `libraries` and `thermostat` to the sketch directory. Open the project file `thermostat_TSensor.ino` in the `Arduino` application. Then compile and upload to Arduino.

### Hardware
* Arduino Nano
* LCD1602 I2C Blue
* Relay TONGLING JQC-3FF-S-Z
* Potentsiometr (variable resistor to adjust the limit temperature)
* 1 buttons (turning on/off the backlight)
* 4 red LEDs (heating progress)

## Works stably
The program works stably and does not freeze. After turning off the power, the device boots up and continues to work.
The Watchdog function is used to handle device freezes. The device will be rebooted if it does not respond within 8 seconds.
The program has been tested and refined for a long time. The repository contains version 12.

## Photos of the working device
![IMG_20231104_110153](https://github.com/neosy/arduino_thermostat/assets/105918329/f894bf71-b588-4114-b1e7-5491affc128f)

## Photos of the prototype
![IMG_20220623_154510](https://github.com/neosy/arduino_thermostat/assets/105918329/1d8c924e-b056-461b-8bd5-d0367d3d5071)
![IMG_20220623_154545](https://github.com/neosy/arduino_thermostat/assets/105918329/f85de1cc-8060-4b1f-b585-50659d84da43)
![IMG_20220623_154550](https://github.com/neosy/arduino_thermostat/assets/105918329/3a7f63af-07ab-48d8-92f8-19d22b17cffb)
