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

	period = 0;
	duty = 0;
	polarity = 0;

	setPeriod(20000000);
	setDuty(10000000);
	setPolarity(1);
}

PWMChannel::PWMChannel() {	// Identifies the correct file path to communicate with PWM via sysfs
	memset(basePath, 0, sizeof(basePath));	// clear path array
	period = 0;
	duty = 0;
	polarity = 0;
}

int PWMChannel::setPeriod(unsigned long p) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(periodPath, O_WRONLY);
	if (fd < 0) {
		cout << "Failed to set channel PWM period!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", p);
	write(fd, buf, len);
	close(fd);

	period = p;

	return 0;
}

int PWMChannel::setDuty(unsigned long d) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(dutyPath, O_WRONLY);
	if (fd < 0) {
		cout << "Failed to set channel PWM duty!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", d);
	write(fd, buf, len);
	close(fd);

	duty = d;

	return 0;
}

int PWMChannel::setPolarity(unsigned long p) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(polarityPath, O_WRONLY);
	if (fd < 0) {
		cout << "Failed to set channel PWM polarity!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", p);
	write(fd, buf, len);
	close(fd);

	polarity = p;

	return 0;
}

int PWMChannel::enable() {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(runPath, O_WRONLY);
	if (fd < 0) {
		cout << "Failed to enable channel PWM!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", 1);
	write(fd, buf, len);
	close(fd);

	return 0;
}

int PWMChannel::disable() {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(runPath, O_WRONLY);
	if (fd < 0) {
		cout << "Failed to disable channel PWM!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", 0);
	write(fd, buf, len);
	close(fd);

	return 0;
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
	throttleTrim = 0;
	pitchTrim = 0;
	rollTrim = 0;
	yawTrim = 0;
	mixMode = mix;

	setFlapMode(mix);
}
/*
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
*/
int aircraftControls::init() {
	throttleChannel = PWMChannel(THROTTLE_HEADER, THROTTLE_PIN);
	elevatorChannel = PWMChannel(ELEVATOR_HEADER, ELEVATOR_PIN);
	aileronChannel = PWMChannel(AILERON_HEADER, AILERON_PIN);
	leftAileronChannel = PWMChannel(LEFT_AILE_HEADER, LEFT_AILE_PIN);
	rightAileronChannel = PWMChannel(RIGHT_AILE_HEADER, RIGHT_AILE_PIN);
	rudderChannel = PWMChannel(RUDDER_HEADER, RUDDER_PIN);

	throttleChannel.setPeriod(20000000);	// 50Hz PWM frequency
	throttleChannel.setDuty(10000000);	// set 50% duty cycle
	throttleChannel.setPolarity(1);
	throttleChannel.enable();	// Enable PWM output

	elevatorChannel.setPeriod(20000000);	// 50Hz PWM frequency
	elevatorChannel.setDuty(10000000);	// set 50% duty cycle
	elevatorChannel.setPolarity(1);
	elevatorChannel.enable();	// Enable PWM output

	aileronChannel.setPeriod(20000000);	// 50Hz PWM frequency
	aileronChannel.setDuty(10000000);	// set 50% duty cycle
	aileronChannel.setPolarity(1);
	aileronChannel.enable();	// Enable PWM output

	leftAileronChannel.setPeriod(20000000);	// 50Hz PWM frequency
	leftAileronChannel.setDuty(10000000);	// set 50% duty cycle
	leftAileronChannel.setPolarity(1);
	leftAileronChannel.enable();	// Enable PWM output

	rightAileronChannel.setPeriod(20000000);	// 50Hz PWM frequency
	rightAileronChannel.setDuty(10000000);	// set 50% duty cycle
	rightAileronChannel.setPolarity(1);
	rightAileronChannel.enable();	// Enable PWM output

	rudderChannel.setPeriod(20000000);	// 50Hz PWM frequency
	rudderChannel.setDuty(10000000);	// set 50% duty cycle
	rudderChannel.setPolarity(1);
	rudderChannel.enable();	// Enable PWM output

	return 0;
}

int aircraftControls::reset() {
	throttleChannel.disable();	// Disable PWM output
	elevatorChannel.disable();	// Disable PWM output
	aileronChannel.disable();	// Disable PWM output
	leftAileronChannel.disable();	// Disable PWM output
	rightAileronChannel.disable();	// Disable PWM output
	rudderChannel.disable();	// Disable PWM output
	throttleTrim = 0;
	pitchTrim = 0;
	rollTrim = 0;
	yawTrim = 0;

	init();

	return 0;
}

int aircraftControls::setFlapMode(FLAP_MIX_MODE mix) {
	mixMode = mix;
	return 0;
}

int aircraftControls::setThrottle(int percent) {
	percent = (percent + 100) / 2;
	percent += throttleTrim;
	throttleChannel.setDuty((float)throttleChannel.getPeriod() * (float)percent / 100);
	return 0;
}

int aircraftControls::setPitch(int percent) {
	return 0;
}

int aircraftControls::setRoll(int percent) {
	return 0;
}

int aircraftControls::setYaw(int percent) {
	percent = (percent + 100) / 2;
	percent += yawTrim;
	rudderChannel.setDuty((float)rudderChannel.getPeriod() * (float)percent / 100);
	return 0;
}

aircraftControls::~aircraftControls() {
	// TODO Auto-generated destructor stub
}
