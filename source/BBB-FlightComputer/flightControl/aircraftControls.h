/*
 * controlSurfaces.h
 *
 *  Created on: Jul 21, 2014
 *      Author: phreaknux
 *
 *     Details: Requires am33xx_pwm device tree overlay enabled
 * Connections:	P9_42 -> Output Channel 1
 */

#ifndef AIRCRAFTCONTROLS_H_
#define AIRCRAFTCONTROLS_H_

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stropts.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

enum TX_CONTROL_CHANNELS {	// Define these according to your transmitter
	TX_CHANNEL_AILERON		= 1,
	TX_CHANNEL_ELEVATOR		= 2,
	TX_CHANNEL_THROTTLE		= 3,
	TX_CHANNEL_RUDDER		= 4
};

enum AIRCRAFT_CONTROL_CHANNELS {	// Define these according to the wiring on your aircraft
	AIRCRAFT_CHANNEL_DUAL_AILERON	= 1,
	AIRCRAFT_CHANNEL_ELEVATOR		= 2,
	AIRCRAFT_CHANNEL_THROTTLE		= 3,
	AIRCRAFT_CHANNEL_RUDDER			= 4,
	AIRCRAFT_CHANNEL_LEFT_AILERON	= 1,	// setup for elevons or individually controlled ailerons
	AIRCRAFT_CHANNEL_RIGHT_AILERON	= 4		// setup for elevons or individually controlled ailerons
};

enum FLAP_MIX_MODE {
	FLAP_MIX_ACRO		= 0,
	FLAP_MIX_ELEVON		= 1
};

class aircraftControls {
private:
	int throttle;	// In + percentage
	int pitch;	// In +/- percentage
	int roll;	// In +/- percentage
	int yaw;	// In +/- percentage
	int fullDeflection;		// degrees
	FLAP_MIX_MODE mixMode;

	int exportPWM();
	int unexportPWM();

	int setPWMDuty(unsigned long duty);
	int setPWMPeriod(unsigned long period);
	int startPWM();
	int stopPWM();
	int setPWMPolarity(int polarity);

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
