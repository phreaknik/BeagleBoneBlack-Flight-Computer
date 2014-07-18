//============================================================================
// Name        : main-hardwareTest.cpp
// Author      : John Boyd
// Version     : 0.1
// Copyright   : is for chumps. This work is free for you to copy.
// Description : This program tests the IMU hardware on the AltIMU-10
//============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "BBB-FlightComputer/hardware/LMS303.h"
#include "BBB-FlightComputer/hardware/LPS331Altimeter.h"
#include "BBB-FlightComputer/hardware/L3GD20Gyro.h"

using namespace std;

unsigned long delta_t = 0;

unsigned long micros();

int main(int argc, char* argv[]) {
	LMS303 lms303(1, 0x1d);
	LPS331Altimeter lps331(1, 0x5d);
	L3GD20Gyro l3gd20(1, 0x6b);

	while(1) {
		lms303.readFullSensorState();
		lps331.readFullSensorState();
		l3gd20.readFullSensorState();

		cout << "##################################\n";
		cout << "Magnetism X:\t" << lms303.getMagX() << " gauss" << endl;
		cout << "Magnetism Y:\t" << lms303.getMagY() << " gauss" << endl;
		cout << "Magnetism Z:\t" << lms303.getMagZ() << " gauss" << endl << endl;

		cout << "Accel X:\t" << lms303.getAccelX() << " g" << endl;
		cout << "Accel Y:\t" << lms303.getAccelY() << " g" << endl;
		cout << "Accel Z:\t" << lms303.getAccelZ() << " g" << endl << endl;

		cout << "Pitch:\t" << lms303.getPitch() << "\u00b0" << endl;
		cout << "Roll:\t" << lms303.getRoll() << "\u00b0" << endl << endl;

		cout << "Core temperature:\t" << lms303.getTemperature() << "\u00b0C" << endl << endl;

		cout << "Pressure:\t" << lps331.getPressure() << " mBar" << endl;
		cout << "Altitude:\t" << lps331.getAltitude() << " m" << endl << endl;

		cout << "Roll X:\t" << l3gd20.getGyroX() << " \u00b0/s" << endl;
		cout << "Roll Y:\t" << l3gd20.getGyroY() << " \u00b0/s" << endl;
		cout << "Roll Z:\t" << l3gd20.getGyroZ() << " \u00b0/s" << endl;

		sleep(1);

	}

	return 0;
}
