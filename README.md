# arduino_thermostat
Arduino-based thermostat.
The thermostat was designed to control an Arduino-based water heater.

## Installation
Copy files from the `lib` and `src` folders to the same directory. Open the project file `thermostat_TSensor.ino` in the `Arduino` application. Then compile and upload to Arduino.

## Works stably
The program works stably and does not freeze. After turning off the power, the device boots up and continues to work.
The Watchdog function is used to handle device freezes. The device will be rebooted if it does not respond within 8 seconds.
The program was tested and refined over 1.5 years.

## Photos of the working device
![IMG_20231104_110153](https://github.com/neosy/arduino_thermostat/assets/105918329/f894bf71-b588-4114-b1e7-5491affc128f)


![IMG_20231104_110144](https://github.com/neosy/arduino_thermostat/assets/105918329/d9c52869-d4b6-4fa2-9622-21f349be7470)
