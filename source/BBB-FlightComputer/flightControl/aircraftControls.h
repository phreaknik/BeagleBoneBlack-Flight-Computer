/*
 * controlSurfaces.h
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
#define THROTTLE_PIN		21
#define ELEVATOR_HEADER		9
#define ELEVATOR_PIN		21
#define AILERON_HEADER		9
#define AILERON_PIN			21
#define LEFT_AILE_HEADER	9
#define LEFT_AILE_PIN		21
#define RIGHT_AILE_HEADER	9
#define RIGHT_AILE_PIN		21
#define RUDDER_HEADER		9
#define RUDDER_PIN			21

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

class PWMChannel {
private:
	// File paths for PWM control
	char basePath[FILE_PATH_LENGTH];
	char periodPath[FILE_PATH_LENGTH];
	char dutyPath[FILE_PATH_LENGTH];
	char polarityPath[FILE_PATH_LENGTH];
	char runPath[FILE_PATH_LENGTH];

	std::string GetFullNameOfFileInDirectory(const std::string & dirName, const std::string & fileNameToFind);

public:
	PWMChannel(int header, int pin);
	PWMChannel();
	char* getPeriodPath() { return periodPath; }
	char* getDutyPath() { return dutyPath; }
	char* getPolarityPath() { return polarityPath; }
	char* getRunPath() { return runPath; }
	virtual ~PWMChannel();
};

class aircraftControls {
public:	// ****** Make private once done with DEBUG
	int throttle;	// In + percentage
	int pitch;	// In +/- percentage
	int roll;	// In +/- percentage
	int yaw;	// In +/- percentage
	int fullDeflection;		// degrees
	FLAP_MIX_MODE mixMode;

	PWMChannel throttleChannel;
	PWMChannel elevatorChannel;
	PWMChannel aileronChannel;
	PWMChannel leftAileronChannel;
	PWMChannel rightAileronChannel;
	PWMChannel rudderChannel;

	//int exportPWM();
	//int unexportPWM();

	int setPWMDuty(PWMChannel channel, unsigned long duty);
	int setPWMPeriod(PWMChannel channel, unsigned long period);
	int startPWM(PWMChannel channel);
	int stopPWM(PWMChannel channel);
	int setPWMPolarity(PWMChannel channel, int polarity);

public:
	aircraftControls(FLAP_MIX_MODE mixMode);
	int init();
	int reset();

	int setFlapMode(FLAP_MIX_MODE mixMode);

	int getThrottle() { return throttle; }
	int getYaw() { return yaw; }
	int getPitch() { return pitch; }
	int getRoll() { return roll; }
	int setThrottle();
	int setPitch();
	int setRoll();

	virtual ~aircraftControls();
};
#endif /* AIRCRAFTCONTROLS_H_ */
