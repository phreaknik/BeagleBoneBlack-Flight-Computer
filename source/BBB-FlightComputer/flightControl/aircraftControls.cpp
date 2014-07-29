/*
 * aircraftControls.cpp
 *
 *  Created on: Jul 21, 2014
 *      Author: phreaknux
 *    Requires: am33xx_pwm and PWM pin specific device tree overlays.
 */

#include "aircraftControls.h"

#define MAX_BUF	64
#define FLAP_DEFLECTION_ANGLE	15	// Max throw of flaps in degrees

using namespace std;

int PWMChannel::loadDeviceTree(int header, int pin) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };

	std::string slotPath = "/sys/devices/";
	slotPath = slotPath + GetFullNameOfFileInDirectory(slotPath, "bone_capemgr.");
	slotPath = slotPath + "/slots";

	// Load global PWM device tree overlay
	int slot = getCapeManagerSlot(header, pin, "am33xx_pwm");

	if(slot == -1) {
		fd = open(slotPath.c_str(), O_WRONLY);
		if (fd < 0) {
			cout << "Failed to load am33xx_pwm device tree overly!" << endl;
			close(fd);
			return -1;
		}
		write(fd, "am33xx_pwm", 10);	// Load global PWM device tree overly
		close(fd);

		// Check of overlay loaded successfully
		slot = getCapeManagerSlot(header, pin, "am33xx_pwm");

		if(slot != -1) {
			cout << "Device tree overlay am33xx_pwm successfully loaded to slot: " << slot << endl;
		}
		else {
			cout << "Device tree overlay am33xx_pwm failed to load!" << endl;
			return -1;
		}
	}
	else {
		cout << "Device tree overlay am33xx_pwm already loaded." << endl;
	}


	// Load pin specific PWM device tree overlay
	len = snprintf(buf, sizeof(buf), "bone_pwm_P%lu_%lu", header, pin);
	slot = getCapeManagerSlot(header, pin, buf);

	if(slot == -1) {
		fd = open(slotPath.c_str(), O_WRONLY);
		if (fd < 0) {
			cout << "Failed to load " << buf << " device tree overly!" << endl;
			close(fd);
			return -1;
		}

		write(fd, buf, len);	// Load global PWM device tree overly
		close(fd);

		// Check of overlay loaded successfully
		slot = getCapeManagerSlot(header, pin, buf);

		if(slot != -1) {
			cout << "Device tree overlay " << buf << " successfully loaded to slot: " << slot << endl;
		}
		else {
			cout << "Device tree overlay " << buf << " failed to load!" << endl;
			return -1;
		}
	}
	else {
		cout << "Device tree overlay " << buf << " already loaded." << endl;
	}

	usleep(500000);	// Give system time to load device tree
	return 0;
}

int PWMChannel::getCapeManagerSlot(int header, int pin, char* name) {
	//cout << " Getting slot!" << endl;
	std::string slotPath = "/sys/devices/";
	slotPath = slotPath + GetFullNameOfFileInDirectory(slotPath, "bone_capemgr.");
	slotPath = slotPath + "/slots";

	ifstream in(slotPath.c_str());
	in.exceptions(std::ios::badbit);
	int slot = -1;
	while (in >> slot)
	{
		string restOfLine;
		getline(in, restOfLine);
		if (restOfLine.find(name) != std::string::npos)
		{
			//cout << "Found device tree overlay " << name << " loaded at slot: " << slot << endl;
			in.close();
			return slot;
		}
	}

	in.close();	// Close file
	return -1;
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

PWMChannel::PWMChannel(int header, int pin) {	// Identifies the correct file path to communicate with PWM via sysfs
	char buf[MAX_BUF] = { 0 };
	// Load PWM device tree overlays
	loadDeviceTree(header, pin);

	// Get sysfs locations to control PWM channel
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

int aircraftControls::init() {
	cout << "\nInitializing PWM channels..." << endl;
	throttleChannel = PWMChannel(THROTTLE_HEADER, THROTTLE_PIN);
	elevatorChannel = PWMChannel(ELEVATOR_HEADER, ELEVATOR_PIN);
	aileronChannel = PWMChannel(AILERON_HEADER, AILERON_PIN);
	leftElevonChannel = PWMChannel(LEFT_ELEVON_HEADER, LEFT_ELEVON_PIN);
	rightElevonChannel = PWMChannel(RIGHT_ELEVON_HEADER, RIGHT_ELEVON_PIN);
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

	leftElevonChannel.setPeriod(20000000);	// 50Hz PWM frequency
	leftElevonChannel.setDuty(10000000);	// set 50% duty cycle
	leftElevonChannel.setPolarity(1);
	leftElevonChannel.enable();	// Enable PWM output

	rightElevonChannel.setPeriod(20000000);	// 50Hz PWM frequency
	rightElevonChannel.setDuty(10000000);	// set 50% duty cycle
	rightElevonChannel.setPolarity(1);
	rightElevonChannel.enable();	// Enable PWM output

	rudderChannel.setPeriod(20000000);	// 50Hz PWM frequency
	rudderChannel.setDuty(10000000);	// set 50% duty cycle
	rudderChannel.setPolarity(1);
	rudderChannel.enable();	// Enable PWM output

	cout << "...Done" << endl;

	return 0;
}

int aircraftControls::reset() {
	throttleChannel.disable();	// Disable PWM output
	elevatorChannel.disable();	// Disable PWM output
	aileronChannel.disable();	// Disable PWM output
	leftElevonChannel.disable();	// Disable PWM output
	rightElevonChannel.disable();	// Disable PWM output
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
	throttleChannel.setDuty((throttleChannel.getPeriod() * percent) / 100);
	return 0;
}

int aircraftControls::setPitch(int percent) {
	//ERRORHERE;
	return 0;
}

int aircraftControls::setRoll(int percent) {
	//ERRORHERE;
	return 0;
}

int aircraftControls::setYaw(int percent) {
	percent = (percent + 100) / 2;
	percent += yawTrim;
	rudderChannel.setDuty((rudderChannel.getPeriod() * percent) / 100);
	return 0;
}

aircraftControls::~aircraftControls() {
	// TODO Auto-generated destructor stub
}
