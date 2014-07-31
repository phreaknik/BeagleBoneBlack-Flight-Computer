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
#define SERVO_MAX_DUTY	2000000
#define SERVO_MIN_DUTY	1000000

using namespace std;

int getCapeManagerSlot(char* name) {
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

std::string GetFullNameOfFileInDirectory(const std::string & dirName, const std::string & fileNameToFind)
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

int loadDeviceTree(int header, int pin) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };

	std::string slotPath = "/sys/devices/";
	slotPath = slotPath + GetFullNameOfFileInDirectory(slotPath, "bone_capemgr.");
	slotPath = slotPath + "/slots";

	// Load pin specific PWM device tree overlay
	len = snprintf(buf, sizeof(buf), "bone_pwm_P%lu_%lu", (unsigned long)header, (unsigned long)pin);
	int slot = getCapeManagerSlot(buf);

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
		slot = getCapeManagerSlot(buf);

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


PWMChannel::PWMChannel(int header, int pin, std::string chName) {	// Identifies the correct file path to communicate with PWM via sysfs
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

	servoMax = SERVO_MAX_DUTY;
	servoMin = SERVO_MIN_DUTY;
	channelName = chName;
	period = 0;
	duty = 0;
	polarity = 0;

	/*	Do this manually later. PWMs are not ready at the point this point
	setPeriod(20000000);
	setDuty(10000000);
	setPolarity(1);
	*/
}

PWMChannel::PWMChannel() {	// Identifies the correct file path to communicate with PWM via sysfs
	memset(basePath, 0, sizeof(basePath));	// clear path array
	servoMax = SERVO_MAX_DUTY;
	servoMin = SERVO_MIN_DUTY;
	period = 0;
	duty = 0;
	polarity = 0;
}

int PWMChannel::setPeriod(unsigned long p) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(periodPath, O_WRONLY);
	if (fd < 0) {
		cout << "Failed to set " << channelName << " PWM period!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", p);
	write(fd, buf, len);
	close(fd);

	period = p;

	return 0;
}

int PWMChannel::setDuty(unsigned long dut) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(dutyPath, O_WRONLY);
	if (fd < 0) {
		cout << "Failed to set " << channelName << " PWM duty!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", dut);
	write(fd, buf, len);
	close(fd);

	duty = dut;

	return 0;
}

int PWMChannel::setPolarity(unsigned long p) {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(polarityPath, O_WRONLY);
	if (fd < 0) {
		cout << "Failed to set " << channelName << " PWM polarity!" << endl;
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
		cout << "Failed to enable " << channelName << " PWM!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", (unsigned long)1);
	write(fd, buf, len);
	close(fd);

	return 0;
}

int PWMChannel::disable() {
	int fd, len;
	char buf[MAX_BUF] = { 0 };	// Data to write
	fd = open(runPath, O_WRONLY);
	if (fd < 0) {
		cout << "Failed to disable " << channelName << " PWM!" << endl;
		close(fd);
		return 1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", (unsigned long)0);
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

	// Create PWM channel objects
	throttleChannel = PWMChannel(THROTTLE_HEADER, THROTTLE_PIN, "throttle channel");
	elevatorChannel = PWMChannel(ELEVATOR_HEADER, ELEVATOR_PIN, "elevator channel");
	aileronChannel = PWMChannel(AILERON_HEADER, AILERON_PIN, "aileron channel");
	leftElevonChannel = PWMChannel(LEFT_ELEVON_HEADER, LEFT_ELEVON_PIN, "left elevon channel");
	rightElevonChannel = PWMChannel(RIGHT_ELEVON_HEADER, RIGHT_ELEVON_PIN, "right elevon channel");
	rudderChannel = PWMChannel(RUDDER_HEADER, RUDDER_PIN, "rudder channel");

	// This function must be called after all PWM channel objects have been instantiated
	PWMInit();

	throttleChannel.setPeriod(25000000);	// 40Hz PWM frequency
	throttleChannel.setDuty(10000000);	// set 50% duty cycle
	throttleChannel.setPolarity(1);
	throttleChannel.enable();	// Enable PWM output

	elevatorChannel.setPeriod(25000000);	// 40Hz PWM frequency
	elevatorChannel.setDuty(10000000);	// set 50% duty cycle
	elevatorChannel.setPolarity(1);
	elevatorChannel.enable();	// Enable PWM output

	aileronChannel.setPeriod(25000000);	// 40Hz PWM frequency
	aileronChannel.setDuty(10000000);	// set 50% duty cycle
	aileronChannel.setPolarity(1);
	aileronChannel.enable();	// Enable PWM output

	leftElevonChannel.setPeriod(25000000);	// 40Hz PWM frequency
	leftElevonChannel.setDuty(10000000);	// set 50% duty cycle
	leftElevonChannel.setPolarity(1);
	leftElevonChannel.enable();	// Enable PWM output

	rightElevonChannel.setPeriod(25000000);	// 40Hz PWM frequency
	rightElevonChannel.setDuty(10000000);	// set 50% duty cycle
	rightElevonChannel.setPolarity(1);
	rightElevonChannel.enable();	// Enable PWM output

	rudderChannel.setPeriod(25000000);	// 40Hz PWM frequency
	rudderChannel.setDuty(10000000);	// set 50% duty cycle
	rudderChannel.setPolarity(1);
	rudderChannel.enable();	// Enable PWM output

	cout << "...Done" << endl;

	return 0;
}

int aircraftControls::PWMInit() {
	int fd;

	std::string slotPath = "/sys/devices/";
	slotPath = slotPath + GetFullNameOfFileInDirectory(slotPath, "bone_capemgr.");
	slotPath = slotPath + "/slots";

	// Load global PWM device tree overlay
	int slot = getCapeManagerSlot((char *)"am33xx_pwm");

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
		slot = getCapeManagerSlot((char *)"am33xx_pwm");

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

	usleep(1000000);
	return 0;
}

/*	A bug in capemgr on the beaglebone causes the device to crash when unloading the PWM. for now, reboot every time.
int aircraftControls::shutdown() {
	int fd;
	char buf[MAX_BUF] = { 0 };

	std::string slotPath = "/sys/devices/";
	slotPath = slotPath + GetFullNameOfFileInDirectory(slotPath, "bone_capemgr.");
	slotPath = slotPath + "/slots";

	// Load global PWM device tree overlay
	int slot = getCapeManagerSlot((char *)"am33xx_pwm");

	if(slot != -1) {
		fd = open(slotPath.c_str(), O_WRONLY);
		if (fd < 0) {
			cout << "Failed to load am33xx_pwm device tree overly!" << endl;
			close(fd);
			return -1;
		}
		snprintf(buf, sizeof(buf), "%d", -1*slot);
		write(fd, buf, 10);	// Load global PWM device tree overly
		close(fd);

		// Check of overlay loaded successfully
		slot = getCapeManagerSlot((char *)"am33xx_pwm");

		if(slot != -1) {
			cout << "Device tree overlay am33xx_pwm successfully loaded to slot: " << slot << endl;
		}
		else {
			cout << "Device tree overlay am33xx_pwm failed to load!" << endl;
			return -1;
		}
	}

	usleep(500000);
	return 0;
}
*/

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
	throttle = percent;
	unsigned long maxDuty = throttleChannel.getServoMax();
	unsigned long minDuty = throttleChannel.getServoMin();
	percent = (percent + 100) / 2;
	percent += throttleTrim;

	unsigned long dutyLevel = ((maxDuty - minDuty) * percent ) / 100;
	dutyLevel += minDuty;
	if(dutyLevel > maxDuty) dutyLevel = maxDuty;
	if(dutyLevel < minDuty) dutyLevel = minDuty;

	throttleChannel.setDuty(dutyLevel);
	return 0;
}

int aircraftControls::setPitch(int percent) {
	pitch = percent;
	percent = (percent + 100) / 2;
	percent += pitchTrim;

	unsigned long maxDuty = rightElevonChannel.getServoMax();
	unsigned long minDuty = rightElevonChannel.getServoMin();

	switch(mixMode) {
	case FLAP_MIX_ACRO: {
		unsigned long dutyLevel = ((maxDuty - minDuty) * percent ) / 100;
		dutyLevel += minDuty;
		if(dutyLevel > maxDuty) dutyLevel = maxDuty;
		if(dutyLevel < minDuty) dutyLevel = minDuty;

		elevatorChannel.setDuty(dutyLevel);
		break;
	}
	case FLAP_MIX_ELEVON: {
		int tempPercent = ((pitch + pitchTrim) / 2) + ((roll + rollTrim) / 2);
		tempPercent = ( tempPercent + 100 ) / 2;
		unsigned long dutyLevel = ((maxDuty - minDuty) * tempPercent ) / 100;
		dutyLevel += minDuty;
		if(dutyLevel > maxDuty) dutyLevel = maxDuty;
		if(dutyLevel < minDuty) dutyLevel = minDuty;

		leftElevonChannel.setDuty(dutyLevel);

		tempPercent = ((pitch + pitchTrim) / 2) - ((roll + rollTrim) / 2);
		tempPercent = ( tempPercent + 100 ) / 2;
		dutyLevel = ((maxDuty - minDuty) * tempPercent ) / 100;
		dutyLevel += minDuty;
		if(dutyLevel > maxDuty) dutyLevel = maxDuty;
		if(dutyLevel < minDuty) dutyLevel = minDuty;

		rightElevonChannel.setDuty(dutyLevel);
		break;
	}
	}

	return 0;
}

int aircraftControls::setRoll(int percent) {
	roll = percent;
	percent = (percent + 100) / 2;
	percent += rollTrim;

	unsigned long maxDuty = rightElevonChannel.getServoMax();
	unsigned long minDuty = rightElevonChannel.getServoMin();

	switch(mixMode) {
	case FLAP_MIX_ACRO: {
		unsigned long dutyLevel = ((maxDuty - minDuty) * percent ) / 100;
		dutyLevel += minDuty;
		if(dutyLevel > maxDuty) dutyLevel = maxDuty;
		if(dutyLevel < minDuty) dutyLevel = minDuty;

		aileronChannel.setDuty(dutyLevel);
		break;
	}
	case FLAP_MIX_ELEVON: {
		int tempPercent = ((pitch + pitchTrim) / 2) + ((roll + rollTrim) / 2);
		tempPercent = ( tempPercent + 100 ) / 2;
		cout << "tempPercent: " << tempPercent << endl;

		unsigned long dutyLevel = ((maxDuty - minDuty) * tempPercent ) / 100;
		dutyLevel += minDuty;
		cout << "dutyLevel: " << dutyLevel << endl;
		if(dutyLevel > maxDuty) dutyLevel = maxDuty;
		if(dutyLevel < minDuty) dutyLevel = minDuty;


		leftElevonChannel.setDuty(dutyLevel);

		tempPercent = ((pitch + pitchTrim) / 2) - ((roll + rollTrim) / 2);
		tempPercent = ( tempPercent + 100 ) / 2;
		cout << "tempPercent: " << tempPercent << endl;

		dutyLevel = ((maxDuty - minDuty) * tempPercent ) / 100;
		dutyLevel += minDuty;
		cout << "dutyLevel: " << dutyLevel << endl;
		if(dutyLevel > maxDuty) dutyLevel = maxDuty;
		if(dutyLevel < minDuty) dutyLevel = minDuty;


		rightElevonChannel.setDuty(dutyLevel);
		break;
	}
	}

	return 0;
}

int aircraftControls::setYaw(int percent) {
	yaw = percent;
	unsigned long maxDuty = rudderChannel.getServoMax();
	unsigned long minDuty = rudderChannel.getServoMin();
	percent = (percent + 100) / 2;
	percent += yawTrim;

	unsigned long dutyLevel = ((maxDuty - minDuty) * percent ) / 100;
	dutyLevel += minDuty;
	if(dutyLevel > maxDuty) dutyLevel = maxDuty;
	if(dutyLevel < minDuty) dutyLevel = minDuty;

	rudderChannel.setDuty(dutyLevel);
	return 0;
}

aircraftControls::~aircraftControls() {
	// TODO Auto-generated destructor stub
}
