/*
 * aircraftControls.h
 *
 *  Created on: Jul 21, 2014
 *      Author: phreaknux
 *
 *     Details: Requires am33xx_pwm device tree overlay enabled
 * Connections:	P9_21 -> Throttle
 */

#ifndef AIRCRAFTCONTROLS_H_
#define AIRCRAFTCONTROLS_H_

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stropts.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

// Define what pins (and headers: P9 or P8) each servo is connected to
#define THROTTLE_HEADER		9
#define THROTTLE_PIN		14
#define ELEVATOR_HEADER		9
#define ELEVATOR_PIN		22
#define AILERON_HEADER		9
#define AILERON_PIN			42
#define LEFT_ELEVON_HEADER	9
#define LEFT_ELEVON_PIN		22
#define RIGHT_ELEVON_HEADER	9
#define RIGHT_ELEVON_PIN	42
#define RUDDER_HEADER		9
#define RUDDER_PIN			42	// Give this channel its own pwm pin

#define FILE_PATH_LENGTH	128

enum TX_CONTROL_CHANNELS {	// Define these according to your transmitter
	TX_CHANNEL_AILERON		= 1,
	TX_CHANNEL_ELEVATOR		= 2,
	TX_CHANNEL_THROTTLE		= 3,
	TX_CHANNEL_RUDDER		= 4
};

enum FLAP_MIX_MODE {
	FLAP_MIX_ACRO		= 0,
	FLAP_MIX_ELEVON		= 1
};

int loadDeviceTree(int header, int pin);
int getCapeManagerSlot(char* name);
std::string GetFullNameOfFileInDirectory(const std::string & dirName, const std::string & fileNameToFind);

class PWMChannel {
private:
	// File paths for PWM control
	std::string channelName;
	char basePath[FILE_PATH_LENGTH];
	char periodPath[FILE_PATH_LENGTH];
	char dutyPath[FILE_PATH_LENGTH];
	char polarityPath[FILE_PATH_LENGTH];
	char runPath[FILE_PATH_LENGTH];

	unsigned long period;
	unsigned long duty;
	unsigned long polarity;
	unsigned long servoMax;
	unsigned long servoMin;

	friend int loadDeviceTree(int header, int pin);
	friend int getCapeManagerSlot(char* name);
	friend std::string GetFullNameOfFileInDirectory(const std::string & dirName, const std::string & fileNameToFind);

public:
	PWMChannel(int header, int pin, std::string chName);
	PWMChannel();
	int init();
	char* getPeriodPath() { return periodPath; }
	char* getDutyPath() { return dutyPath; }
	char* getPolarityPath() { return polarityPath; }
	char* getRunPath() { return runPath; }

	unsigned long getPeriod() { return period; }
	int setPeriod(unsigned long p);
	unsigned long getDuty() { return duty; }
	int setDuty(unsigned long dut);
	unsigned long getPolarity() { return polarity; }
	int setPolarity(unsigned long p);
	unsigned long getServoMax() { return servoMax; }
	unsigned long getServoMin() { return servoMin; }
	int enable();
	int disable();

	virtual ~PWMChannel();
};

class aircraftControls {
public:	// MAKE THIS PRIVATE************************************************************************************
	int throttle;	// In + percentage
	int pitch;	// In +/- percentage
	int roll;	// In +/- percentage
	int yaw;	// In +/- percentage
	int fullDeflection;		// degrees
	int throttleTrim;
	int pitchTrim;
	int rollTrim;
	int yawTrim;
	FLAP_MIX_MODE mixMode;

	PWMChannel throttleChannel;
	PWMChannel elevatorChannel;
	PWMChannel aileronChannel;
	PWMChannel leftElevonChannel;
	PWMChannel rightElevonChannel;
	PWMChannel rudderChannel;

	friend int getCapeManagerSlot(char* name);
	friend std::string GetFullNameOfFileInDirectory(const std::string & dirName, const std::string & fileNameToFind);

public:
	aircraftControls(FLAP_MIX_MODE mix);
	int init();
	int PWMInit();
	//int shutdown();	A bug in capemgr is causing the capemgr to crash when unloading PWM overlays
	int reset();

	int setFlapMode(FLAP_MIX_MODE mix);

	int getThrottle() { return throttle; }
	int getYaw() { return yaw; }
	int getPitch() { return pitch; }
	int getRoll() { return roll; }
	int setThrottle(int percent);
	int setPitch(int percent);
	int setRoll(int percent);
	int setYaw(int percent);

	virtual ~aircraftControls();
};
#endif /* AIRCRAFTCONTROLS_H_ */
