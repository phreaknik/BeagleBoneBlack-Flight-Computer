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
#include "hardware/LMS303.h"
#include "hardware/LPS331Altimeter.h"
#include "hardware/L3GD20Gyro.h"
#include "AHRS/ahrs.h"

using namespace std;

unsigned long delta_t = 0;

unsigned long micros();

int main(int argc, char* argv[]) {
	LMS303 lms303(1, 0x1d);
	LPS331Altimeter alt(1, 0x5d);
	L3GD20Gyro gyro(1, 0x6b);

	// AHRS initialization
	imu::Vector<3> acc = lms303.read_acc();
	imu::Vector<3> mag = lms303.read_mag();
	uimu_ahrs_init(acc, mag);
	uimu_ahrs_set_beta(0.1);

	while(1) {
		// execute at ~50Hz
		unsigned long time = micros();
		if((time - delta_t) < 20000) continue; // Wont let execution pass this line until 20ms has passed
		delta_t = time;

		lms303.readFullSensorState();
		gyro.readFullSensorState();
		alt.readFullSensorState();
		uimu_ahrs_iterate(lms303.read_acc(), gyro.read_gyro(), lms303.read_mag());

		imu::Vector<3> euler = uimu_ahrs_get_euler();
		cout<< "euler: " << euler.x() << " " << euler.y() << " " << euler.z() << "\n";
	}

	return 0;
}

unsigned long micros() {
	timespec timeStamp;
	clock_gettime(CLOCK_MONOTONIC, &timeStamp);
	return timeStamp.tv_nsec / 1000;
}
