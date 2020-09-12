# DHT22.c
C program to read DHT22 sensor on Raspberry Pi ZERO and RaspiOS 10 (Buster).  (Should work with any version of Raspberry Pi.)
Comments and meaningful variable names help explain how program works.  (Requires wiringPi library.)

Uses Temperature and Realitive Humidity to calculate Heat Index using NOAA formula and Dew Point.
Returns a chached value of last good read of Temperature & Humidity if there is an issue getting data.  
(Readings are dependent on NanoSecond timing of state changes in data line.)
