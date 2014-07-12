//============================================================================
// Name        : main.cpp
// Author      : John Boyd
// Version     :
// Copyright   : is for chumps. This work is free for you to copy.
// Description : Main function for Beaglebone Black flight computer.
//============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "LMS303.h"
#include "LPS331Altimeter.h"
#include "L3GD20Gyro.h"

using namespace std;

int main(int argc, char* argv[]) {
	LMS303 accel1(1, 0x1d);
	LPS331Altimeter alt1(1, 0x5d);
	L3GD20Gyro gyro1(1, 0x6b);

	while(1) {
		accel1.readFullSensorState();
		alt1.readFullSensorState();
		gyro1.readFullSensorState();

		cout << "##################################\n";
		cout << "Magnetism X:\t" << accel1.getMagX() << " gauss" << endl;
		cout << "Magnetism Y:\t" << accel1.getMagY() << " gauss" << endl;
		cout << "Magnetism Z:\t" << accel1.getMagZ() << " gauss" << endl << endl;

		cout << "Accel X:\t" << accel1.getAccelX() << " g" << endl;
		cout << "Accel Y:\t" << accel1.getAccelY() << " g" << endl;
		cout << "Accel Z:\t" << accel1.getAccelZ() << " g" << endl << endl;

		cout << "Pitch:\t" << accel1.getPitch() << "\u00b0" << endl;
		cout << "Roll:\t" << accel1.getRoll() << "\u00b0" << endl << endl;

		cout << "Core temperature:\t" << accel1.getTemperature() << "\u00b0" << endl << endl;

		cout << "Pressure:\t" << alt1.getPressure() << " mBar" << endl;
		cout << "Altitude:\t" << alt1.getAltitude() << " m" << endl << endl;

		cout << "Roll X:\t" << gyro1.getGyroX() << " \u00b0/s" << endl;
		cout << "Roll Y:\t" << gyro1.getGyroY() << " \u00b0/s" << endl;
		cout << "Roll Z:\t" << gyro1.getGyroZ() << " \u00b0/s" << endl;

		sleep(1);
	}

	return 0;
}
