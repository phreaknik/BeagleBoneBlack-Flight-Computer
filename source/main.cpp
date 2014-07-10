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

		cout << accel0.getAccelX() << endl;
		cout << accel0.getAccelY() << endl;
		cout << accel0.getAccelZ() << endl;

		cout << "Pitch:\t" << accel0.getPitch() << endl;
		cout << "Roll:\t" << accel0.getRoll() << endl;

		sleep(1);
	}

	return 0;
}
