BeagleBoneBlack_Flight_Computer
===============================

A flight computer for small aircraft based on the BeagleBone Black.

Documentation
-------------
This is compiled using the gnueabihf C++ cross compiler found in Texas Instruments' am335xSDK kit (Linux EZSDK: http://www.ti.com/tool/linuxezsdk-sitara).

The sensors (AltIMU-10 & Ultimate GPS) must be connected to the appropriate BeagleBone Black pins that interface with the I2C bus defined in each sensor's constructor in "source/main.cpp" (Eg. LMS303
lms303(1, 0x1d); // create sensor on I2C bus 1 with address 0x1d

Aircraft servos and ESCs must be connected to the appropriate BeagleBone Black pins as defined in "source/BBB-FlightComputer/flightControls/aircraftControls.h"

Hardware
--------
- AltIMU-10 from Pololu (http://www.pololu.com/product/1269)
- Ultimate GPS Breakout from Adafruit (http://www.adafruit.com/products/746)
- Misc servos and ESCs for aircraft control
