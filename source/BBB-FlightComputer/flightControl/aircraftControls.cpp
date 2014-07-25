/*
 * controlSurfaces.cpp
 *
 *  Created on: Jul 21, 2014
 *      Author: phreaknux
 *    Requires: am33xx_pwm and PWM pin specific device tree overlays.
 */

#include "aircraftControls.h"

#define MAX_BUF	64
#define FLAP_DEFLECTION_ANGLE	15	// Max throw of flaps in degrees

using namespace std;

PWMChannel::PWMChannel(int header, int pin) {	// Identifies the correct file path to communicate with PWM via sysfs
	char buf[MAX_BUF] = { 0 };
	std::string base = "/sys/devices/";
	std::string temp = GetFullNameOfFileInDirectory(base, "ocp.");
	snprintf(buf, sizeof(buf), "pwm_test_P%d_%d.", header, pin);
	temp = base + temp + "/";
	temp = temp + GetFullNameOfFileInDirectory(temp, std::string(buf));
	temp = temp + "/";
	memset(basePath, 0, sizeof(basePath));	// clear path array first
	memcpy(basePath, temp.c_str(), temp.size());

	std::string temp1 = temp + "period";
	memset(periodPath, 0, sizeof(periodPath));	// clear path array first
	memcpy(periodPath, temp1.c_str(), temp1.size());
	//cout << "periodPath: " << periodPath << endl;

	temp1 = temp + "duty";
	memset(dutyPath, 0, sizeof(dutyPath));	// clear path array first
	memcpy(dutyPath, temp1.c_str(), temp1.size());
	//cout << "dutyPath: " << dutyPath << endl;

	temp1 = temp + "polarity";
	memset(polarityPath, 0, sizeof(polarityPath));	// clear path array first
	memcpy(polarityPath, temp1.c_str(), temp1.size());
	//cout << "polarityPath: " << polarityPath << endl;

	temp1 = temp + "run";
	memset(runPath, 0, sizeof(runPath));	// clear path array first
	memcpy(runPath, temp1.c_str(), temp1.size());
	//cout << "runPath: " << runPath << endl;
}

PWMChannel::PWMChannel() {	// Identifies the correct file path to communicate with PWM via sysfs
	memset(basePath, 0, sizeof(basePath));	// clear path array
}

std::string PWMChannel::GetFullNameOfFileInDirectory(const std::string & dirName, const std::string & fileNameToFind)
{
	DIR *pDir;

	dirent *pFile;
	if ((pDir = opendir(dirName.c_str())) == NULL)
	{
		std::cout << "Directory name: " << dirName << " doesnt exist!" << std::endl;
		throw std::bad_exception();
	}
	while ((pFile = readdir(pDir)) != NULL)
	{
		std::string currentFileName = (pFile->d_name);
		if (currentFileName.find(fileNameToFind) != std::string::npos)
		{
			return currentFileName;
		}
	}
	return std::string("");
}

PWMChannel::~PWMChannel() {
	// TODO Auto-generated destructor stub
}

aircraftControls::aircraftControls(FLAP_MIX_MODE mix) {
	throttle = 0;	// In + percentage
	pitch = 0;	// In +/- percentage
	roll = 0;	// In +/- percentage
	yaw = 0;	// In +/- percentage
	fullDeflection = FLAP_DEFLECTION_ANGLE;		// degrees
	mixMode = mix;

	setFlapMode(mix);
}

/*
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
*/

int aircraftControls::setPWMDuty(PWMChannel channel, unsigned long duty) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(channel.getDutyPath(), O_WRONLY);
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

int aircraftControls::setPWMPeriod(PWMChannel channel, unsigned long period) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(channel.getPeriodPath(), O_WRONLY);
	if (fd < 0) {
		cout << "Failed to set PWM period!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", period);
	write(fd, buf, len);
	close(fd);

	return 0;
}

int aircraftControls::startPWM(PWMChannel channel) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(channel.getRunPath(), O_WRONLY);
	if (fd < 0) {
		cout << "Failed to start PWM!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", 1);
	write(fd, buf, len);
	close(fd);

	return 0;
}

int aircraftControls::stopPWM(PWMChannel channel) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(channel.getRunPath(), O_WRONLY);
	if (fd < 0) {
		cout << "Failed to stop PWM!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", 0);
	write(fd, buf, len);
	close(fd);

	return 0;
}

int aircraftControls::setPWMPolarity(PWMChannel channel, int polarity) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(channel.getPolarityPath(), O_WRONLY);
	if (fd < 0) {
		cout << "Failed to set PWM polarity!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", polarity);
	write(fd, buf, len);
	close(fd);

	return 0;
}

int aircraftControls::init() {
	throttleChannel = PWMChannel(THROTTLE_HEADER, THROTTLE_PIN);
	elevatorChannel = PWMChannel(ELEVATOR_HEADER, ELEVATOR_PIN);
	aileronChannel = PWMChannel(AILERON_HEADER, AILERON_PIN);
	leftAileronChannel = PWMChannel(LEFT_AILE_HEADER, LEFT_AILE_PIN);
	rightAileronChannel = PWMChannel(RIGHT_AILE_HEADER, RIGHT_AILE_PIN);
	rudderChannel = PWMChannel(RUDDER_HEADER, RUDDER_PIN);

	setPWMPeriod(throttleChannel, 20000000);	// 50Hz PWM frequency
	setPWMDuty(throttleChannel, 10000000);	// set 50% duty cycle
	setPWMPolarity(throttleChannel, 1);
	startPWM(throttleChannel);	// Enable PWM output

	setPWMPeriod(elevatorChannel, 20000000);	// 50Hz PWM frequency
	setPWMDuty(elevatorChannel, 10000000);	// set 50% duty cycle
	setPWMPolarity(elevatorChannel, 1);
	startPWM(elevatorChannel);	// Enable PWM output

	setPWMPeriod(aileronChannel, 20000000);	// 50Hz PWM frequency
	setPWMDuty(aileronChannel, 10000000);	// set 50% duty cycle
	setPWMPolarity(aileronChannel, 1);
	startPWM(aileronChannel);	// Enable PWM output

	setPWMPeriod(leftAileronChannel, 20000000);	// 50Hz PWM frequency
	setPWMDuty(leftAileronChannel, 10000000);	// set 50% duty cycle
	setPWMPolarity(leftAileronChannel, 1);
	startPWM(leftAileronChannel);	// Enable PWM output

	setPWMPeriod(rightAileronChannel, 20000000);	// 50Hz PWM frequency
	setPWMDuty(rightAileronChannel, 10000000);	// set 50% duty cycle
	setPWMPolarity(rightAileronChannel, 1);
	startPWM(rightAileronChannel);	// Enable PWM output

	setPWMPeriod(rudderChannel, 20000000);	// 50Hz PWM frequency
	setPWMDuty(rudderChannel, 10000000);	// set 50% duty cycle
	setPWMPolarity(rudderChannel, 1);
	startPWM(rudderChannel);	// Enable PWM output

	return 0;
}

int aircraftControls::reset() {
	stopPWM(throttleChannel);	// Disable PWM output
	stopPWM(elevatorChannel);	// Disable PWM output
	stopPWM(aileronChannel);	// Disable PWM output
	stopPWM(leftAileronChannel);	// Disable PWM output
	stopPWM(rightAileronChannel);	// Disable PWM output
	stopPWM(rudderChannel);	// Disable PWM output

	setPWMPeriod(throttleChannel, 20000000);	// 50Hz PWM frequency
	setPWMDuty(throttleChannel, 10000000);	// set 50% duty cycle
	setPWMPolarity(throttleChannel, 1);
	startPWM(throttleChannel);	// Enable PWM output

	setPWMPeriod(elevatorChannel, 20000000);	// 50Hz PWM frequency
	setPWMDuty(elevatorChannel, 10000000);	// set 50% duty cycle
	setPWMPolarity(elevatorChannel, 1);
	startPWM(elevatorChannel);	// Enable PWM output

	setPWMPeriod(aileronChannel, 20000000);	// 50Hz PWM frequency
	setPWMDuty(aileronChannel, 10000000);	// set 50% duty cycle
	setPWMPolarity(aileronChannel, 1);
	startPWM(aileronChannel);	// Enable PWM output

	setPWMPeriod(leftAileronChannel, 20000000);	// 50Hz PWM frequency
	setPWMDuty(leftAileronChannel, 10000000);	// set 50% duty cycle
	setPWMPolarity(leftAileronChannel, 1);
	startPWM(leftAileronChannel);	// Enable PWM output

	setPWMPeriod(rightAileronChannel, 20000000);	// 50Hz PWM frequency
	setPWMDuty(rightAileronChannel, 10000000);	// set 50% duty cycle
	setPWMPolarity(rightAileronChannel, 1);
	startPWM(rightAileronChannel);	// Enable PWM output

	setPWMPeriod(rudderChannel, 20000000);	// 50Hz PWM frequency
	setPWMDuty(rudderChannel, 10000000);	// set 50% duty cycle
	setPWMPolarity(rudderChannel, 1);
	startPWM(rudderChannel);	// Enable PWM output

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
