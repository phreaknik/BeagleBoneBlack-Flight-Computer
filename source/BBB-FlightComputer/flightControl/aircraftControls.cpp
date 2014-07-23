/*
 * controlSurfaces.cpp
 *
 *  Created on: Jul 21, 2014
 *      Author: phreaknux
 */

#include "aircraftControls.h"

#define MAX_BUF	64
#define FLAP_DEFLECTION_ANGLE	15	// Max throw of flaps in degrees

using namespace std;

aircraftControls::aircraftControls(FLAP_MIX_MODE mix) {
	throttle = 0;	// In + percentage
	pitch = 0;	// In +/- percentage
	roll = 0;	// In +/- percentage
	yaw = 0;	// In +/- percentage
	fullDeflection = FLAP_DEFLECTION_ANGLE;		// degrees
	mixMode = mix;

	setFlapMode(mix);
}

int aircraftControls::exportPWM() {
	int fd, len;
	char buf[MAX_BUF] = { 0 };
	struct stat st; //check if already exported...just checking one pwm

	fd = open("/sys/class/pwm/export", O_WRONLY);
	if(fd < 0) cout << "Failed to export PWMs!" << endl;

	if(stat("/sys/class/pwm/pwm0",&st)==0) cout << "PWM0 already exported";
	else {
		len = snprintf(buf, sizeof(buf), "%d", 0);
		write(fd, buf, len);
	}

	close(fd);
	return 0;
}

int aircraftControls::unexportPWM() {
	int fd, len;
	char buf[MAX_BUF] = { 0 };
	struct stat st; //check if already exported...just checking one pwm

	fd = open("/sys/class/pwm/unexport", O_WRONLY);
	if(fd < 0) cout << "Failed to unexport PWMs!" << endl;

	if(stat("/sys/class/pwm/pwm0",&st)==0) cout << "PWM0 already unexported";
	else {
		len = snprintf(buf, sizeof(buf), "%d", 0);
		write(fd, buf, len);
	}

	close(fd);
	return 0;
}

int aircraftControls::setPWMDuty(unsigned long duty) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };
	fd = open("/sys/class/pwm/pwm0/duty_ns", O_WRONLY);
	if (fd < 0) {
		cout << "Failed to set PWM duty!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", duty);
	write(fd, buf, len);
	close(fd);

	return 0;
}

int aircraftControls::setPWMPeriod(unsigned long period) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };

	snprintf(buf, sizeof(buf), "/sys/class/pwm/pwm%d/period_ns", 0);
	fd = open(buf, O_WRONLY);
	if(fd < 0) {
		cout << "Failed to set PWM period!" << endl;
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", period); //20ms period for servo
	write(fd, buf, len);
	close(fd);

	return 0;
}

int aircraftControls::startPWM() {
	int fd, len;
	char buf[MAX_BUF] = { 0 };
	snprintf(buf, sizeof(buf), "/sys/class/pwm/pwm%d/run", 0);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		cout << "Failed to enable PWM!" << endl;
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%d", 1);
	write(fd, buf, len);
	close(fd);

	return 0;
}

int aircraftControls::stopPWM() {
	int fd, len;
	char buf[MAX_BUF] = { 0 };
	snprintf(buf, sizeof(buf), "/sys/class/pwm/pwm%d/run", 0);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		cout << "Failed to enable PWM!" << endl;
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%d", 0);
	write(fd, buf, len);
	close(fd);

	return 0;
}

int aircraftControls::setPWMPolarity(int polarity) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };
	snprintf(buf, sizeof(buf), "/sys/class/pwm/pwm%d/polarity", 0);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		cout << "Failed to set PWM polarity!" << endl;
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%d", polarity);
	write(fd, buf, len);
	close(fd);

	return 0;
}

int aircraftControls::init() {
	exportPWM();
	setPWMDuty(10000000);
	setPWMPolarity(1);
	setPWMPeriod(20000000);	// 50Hz PWM frequency
	startPWM();	// Enable PWM output
	return 0;
}

int aircraftControls::reset() {
	stopPWM();
	unexportPWM();

	return 0;
}

int aircraftControls::setFlapMode(FLAP_MIX_MODE mixMode) {
	return 0;
}

int aircraftControls::setThrottle() {
	return 0;
}

int aircraftControls::setPitch() {
	return 0;
}

int aircraftControls::setRoll() {
	return 0;
}

aircraftControls::~aircraftControls() {
	// TODO Auto-generated destructor stub
}
