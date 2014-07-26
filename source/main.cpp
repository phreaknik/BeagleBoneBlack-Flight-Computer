//============================================================================
// Name        : main.cpp
// Author      : John Boyd
// Version     :
// Copyright   : This work is free for you to copy.
// Description : Main function for Beaglebone Black flight computer.
// Resources   : PRU - https://github.com/beagleboard/am335x_pru_package
//============================================================================

#include "BBB-FlightComputer/BBB-FlightComputer.h"

unsigned long delta_t;

using namespace std;

int main(int argc, char* argv[]) {
	/* Experimental Quaternion based AHRS
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
		usleep(200000);	// Actually seems to delay nanoseconds
		//unsigned long time = micros();
		//if((time - delta_t) < 20000) continue; // Wont let execution pass this line until 20ms has passed
		//cout << "Delta T: " << time - delta_t << endl;
		//delta_t = time;

		lms303.readFullSensorState();
		gyro.readFullSensorState();
		alt.readFullSensorState();
		uimu_ahrs_iterate(gyro.read_gyro(), lms303.read_acc(), lms303.read_mag());

		imu::Vector<3> euler = uimu_ahrs_get_euler();
		cout<< "euler: " << euler.x() << " " << euler.y() << " " << euler.z() << "\n";
	}
	*/

	// Sensors
	LMS303 lms303(1, 0x1d);
	LPS331Altimeter lps331(1, 0x5d);
	L3GD20Gyro l3gd20(1, 0x6b);

	// Aircraft
	aircraftControls aircraft(FLAP_MIX_ELEVON);
	aircraft.init();

	while(1) {
		aircraft.setThrottle(-100);
		usleep(2000000);

		aircraft.setThrottle(-50);
		usleep(2000000);

		aircraft.setThrottle(0);
		usleep(2000000);

		aircraft.setThrottle(50);
		usleep(2000000);

		aircraft.setThrottle(100);
		usleep(2000000);
	}

	/*
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

		usleep(500000);

	} // \Hardware test
	*/
	return 0;
}
