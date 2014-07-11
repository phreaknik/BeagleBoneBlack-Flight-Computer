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

using namespace std;

int main(int argc, char* argv[]) {
	LMS303 accel0(1, 0x1d);

	while(1) {
		accel0.readFullSensorState();

		cout << "##################################\n";
		cout << "Magnetism X:\t" << accel0.getMagX() << endl;
		cout << "Magnetism Y:\t" << accel0.getMagY() << endl;
		cout << "Magnetism Z:\t" << accel0.getMagZ() << endl << endl;

		cout << "Accel X:\t" << accel0.getAccelX() << endl;
		cout << "Accel Y:\t" << accel0.getAccelY() << endl;
		cout << "Accel Z:\t" << accel0.getAccelZ() << endl << endl;

		cout << "Pitch:\t" << accel0.getPitch() << "\u00b0" << endl;
		cout << "Roll:\t" << accel0.getRoll() << "\u00b0" << endl << endl;

		cout << "Core temperature:\t" << accel0.getTemperature() << "\u00b0" << endl;

		sleep(1);
	}

	return 0;
}
