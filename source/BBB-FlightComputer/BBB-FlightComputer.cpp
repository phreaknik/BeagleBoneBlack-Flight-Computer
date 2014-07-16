//============================================================================
// Name        : BBB-FlightComputer.cpp
// Author      : John Boyd
// Version     :
// Copyright   : This work is free for you to copy.
// Description : This file defines some generic functions used in the
//				 BeagleBone Black Flight Computer.
//============================================================================

#include "BBB-FlightComputer.h"

unsigned long micros() {
    timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return (tv.tv_sec) * 1000000 + (tv.tv_nsec)/1000000;
}
