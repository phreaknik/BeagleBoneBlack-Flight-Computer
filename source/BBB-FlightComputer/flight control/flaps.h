/*
 * hardware.h
 *
 *  Created on: Jul 14, 2014
 *      Author: phreaknux
 */

#ifndef FLAPS_H_
#define FLAPS_H_

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stropts.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

enum FLAP_MODE {
	FLAP_MODE_ACRO		= 0,
	FLAP_MODE_ELEVON	= 1
};

class LMS303 {
private:
	float pitch;	// In percentage
	float roll;		// In percentage
	float fullDeflection;		// In degrees

public:
	int reset();

	int setFlapMode(LMS303_MAG_SCALE scale);

	float getPitch() { return pitch; }
	float getRoll() { return roll; }
	float setPitch();
	float setRoll();

	virtual ~LMS303();
};

#endif /* FLAPS_H_ */
