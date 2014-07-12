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

using namespace std;

int main(int argc, char* argv[]) {
	LMS303 accel0(1, 0x1d);
	LPS331Altimeter alt0(1, 0x5d);

	while(1) {
		accel0.readFullSensorState();
		alt0.readFullSensorState();

		cout << "##################################\n";
		cout << "Magnetism X:\t" << accel0.getMagX() << " gauss" << endl;
		cout << "Magnetism Y:\t" << accel0.getMagY() << " gauss" << endl;
		cout << "Magnetism Z:\t" << accel0.getMagZ() << " gauss" << endl << endl;

		cout << "Accel X:\t" << accel0.getAccelX() << " g" << endl;
		cout << "Accel Y:\t" << accel0.getAccelY() << " g" << endl;
		cout << "Accel Z:\t" << accel0.getAccelZ() << " g" << endl << endl;

		cout << "Pitch:\t" << accel0.getPitch() << "\u00b0" << endl;
		cout << "Roll:\t" << accel0.getRoll() << "\u00b0" << endl << endl;

		cout << "Core temperature:\t" << accel0.getTemperature() << "\u00b0" << endl << endl;

		cout << "Pressure:\t" << alt0.getPressure() << " mBar" << endl;
		cout << "Pressure:\t" << alt0.getAltitude() << " m" << endl;

		sleep(1);
	}

	return 0;
}
