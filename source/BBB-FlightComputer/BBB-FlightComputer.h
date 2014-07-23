//============================================================================
// Name        : BBB-FlightComputer.h
// Author      : John Boyd
// Version     :
// Copyright   : This work is free for you to copy.
// Description : This file contains all resources for the Beaglebone Black
// 				 flight computer.
//============================================================================

#ifndef BBB_FLIGHTCOMPUTER_H_
#define BBB_FLIGHTCOMPUTER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "sensors/LMS303.h"
#include "sensors/LPS331Altimeter.h"
#include "sensors/L3GD20Gyro.h"
#include "AHRS/ahrs.h"
#include "flightControl/aircraftControls.h"
#include <time.h>

unsigned long micros();


#endif /* BBB_FLIGHTCOMPUTER_H_ */
