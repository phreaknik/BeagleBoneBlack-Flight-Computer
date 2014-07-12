#ifndef UIMU_AHRS_H
#define UIMU_AHRS_H

#include "imumaths.h"
#include <time.h>

//initialises the AHRS
void uimu_ahrs_init(imu::Vector<3> acc, imu::Vector<3> mag);

void uimu_ahrs_set_offset(imu::Quaternion o);

//sets the beta. this controls how strong the drift correction will be.
//a higher beta means more correction
void uimu_ahrs_set_beta(float beta);


//does an iteration. call this every 20ms at least
void uimu_ahrs_iterate(imu::Vector<3> acc, imu::Vector<3> ang_vel, imu::Vector<3> mag);

//returns the orientation in various forms
imu::Vector<3> uimu_ahrs_get_euler(); //heading, pitch, roll in degrees
imu::Matrix<3> uimu_ahrs_get_matrix(); //north-east-down rotation matrix
imu::Quaternion uimu_ahrs_get_quaternion(); //good ol' quaternion

imu::Quaternion uimu_ahrs_get_imu_quaternion();


#endif
