/*
 * LMS303.h
 *	For use with LMS303 Accelerometer as found in AltIMU-10. Note, must set dataRate to enable
 *	measurements.
 *
 *  Created on: Jul 3, 2014
 *      Author: John Boyd
 *
 *  Reference:
 *  	http://www.inmotion.pt/store/altimu10-v3-gyro-accelerometer-compass-and-altimeter-l3gd20h
 */

#ifndef LMS303_H_
#define LMS303_H_

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
#include "../AHRS/imumaths.h"

#define LMS303_I2C_BUFFER		0x40	// Only 0x40 registers available according to LMS303 datasheet
#define ACCEL_FIFO_SLOTS				0x0F	// Number of slots in FIFO for each accelerometer output
#define ACCEL_FIFO_SIZE 				0x60	// Size of FIFO array
#define MAX_BUS					64

#define REG_TEMP_OUT_L			0x05
#define REG_TEMP_OUT_H			0x06
#define REG_STATUS_M			0x07
#define REG_OUT_X_L_M			0x08
#define REG_OUT_X_H_M			0x09
#define REG_OUT_Y_L_M			0x0A
#define REG_OUT_Y_H_M			0x0B
#define REG_OUT_Z_L_M			0x0C
#define REG_OUT_Z_H_M			0x0D
#define REG_WHO_AM_I			0x0F
#define REG_INT_CTRL_M			0x12
#define REG_CTRL0				0x1F
#define REG_CTRL1				0x20
#define REG_CTRL2				0x21
#define REG_CTRL3				0x22
#define REG_CTRL4				0x23
#define REG_CTRL5				0x24
#define REG_CTRL6				0x25
#define REG_CTRL7				0x26
#define REG_STATUS_A			0x27
#define REG_OUT_X_L_A			0x28
#define REG_OUT_X_H_A			0x29
#define REG_OUT_Y_L_A			0x2A
#define REG_OUT_Y_H_A			0x2B
#define REG_OUT_Z_L_A			0x2C
#define REG_OUT_Z_H_A			0x2D
#define REG_FIFO_CTRL			0x2E
#define REG_FIFO_SRC			0x2F
#define REG_IG_CFG1				0x30

enum LMS303_MAG_SCALE {
	SCALE_MAG_2gauss	= 0,
	SCALE_MAG_4gauss	= 1,
	SCALE_MAG_8gauss	= 2,
	SCALE_MAG_12gauss	= 3
};

enum LMS303_MAG_DATA_RATE {
	DR_MAG_3p125HZ		= 0,
	DR_MAG_6p25HZ		= 1,
	DR_MAG_12p5HZ		= 2,
	DR_MAG_25HZ			= 3,
	DR_MAG_50HZ			= 4,
	DR_MAG_100HZ		= 5	// 100HZ BW only available if Accel BW > 50Hz or = 0
};

enum LMS303_ACCEL_SCALE {
	SCALE_ACCEL_2g		= 0,
	SCALE_ACCEL_4g		= 1,
	SCALE_ACCEL_6g		= 2,
	SCALE_ACCEL_8g		= 3,
	SCALE_ACCEL_16g		= 4
};

enum LMS303_ACCEL_DATA_RATE {
	DR_ACCEL_SHUTDOWN	= 0,
	DR_ACCEL_3p125HZ 	= 1,
	DR_ACCEL_6p25HZ 	= 2,
	DR_ACCEL_12p5HZ 	= 3,
	DR_ACCEL_25HZ 		= 4,
	DR_ACCEL_50HZ 		= 5,
	DR_ACCEL_100HZ 		= 6,
	DR_ACCEL_200HZ 		= 7,
	DR_ACCEL_4OOHZ 		= 8,
	DR_ACCEL_8OOHZ 		= 9,
	DR_ACCEL_16OOHZ 	= 10,
	DR_ACCEL_ERROR		= -1
};

enum LMS303_ACCEL_FIFO_MODE {
	ACCEL_FIFO_BYPASS,
	ACCEL_FIFO_STREAM,	// This mode enables averaging
	ACCEL_FIFO_ERROR
};

class LMS303 {

private:
	float celsius;

	int I2CBus, I2CAddress;
	char dataBuffer[LMS303_I2C_BUFFER];
	char accelFIFO[ACCEL_FIFO_SIZE];	// 16 FIFO slots * 6 Accel output registers
	LMS303_ACCEL_FIFO_MODE accelFIFOMode = ACCEL_FIFO_BYPASS;

	double magScale;
	float magX;
	float magY;
	float magZ;

	double accelScale;
	float accelX;	// in g's
	float accelY;	// in g's
	float accelZ;	// in g's

	double pitch;	// in degrees
	double roll;	// in degrees

	int writeI2CDeviceByte(char address, char value);
	int readI2CDevice(char address, char data[], int size);

	float convertMagnetism(int msb_reg_addr, int lsb_reg_addr);

	float convertAcceleration(int msb_reg_addr, int lsb_reg_addr);
	float convertAcceleration(int accel);
	void calculatePitchAndRoll();
	int readAccelFIFO(char buffer[]);
	int averageAccelFIFO(int slots);

public:

	LMS303(int bus, int address);

	int reset();
	int readFullSensorState();

	int enableTempSensor();
	int getTemperature();

	int enableMagnetometer();
	float getMagX() { return magX; }
	float getMagY() { return magY; }
	float getMagZ() { return magZ; }
	int setMagScale(LMS303_MAG_SCALE scale);
	int setMagDataRate(LMS303_MAG_DATA_RATE dataRate);

	int enableAccelerometer();
	int setAccelScale(LMS303_ACCEL_SCALE scale);
	int setAccelDataRate(LMS303_ACCEL_DATA_RATE dataRate);	// Must set dataRate to enable device
	LMS303_ACCEL_DATA_RATE getAccelDataRate();
	int setAccelFIFOMode(LMS303_ACCEL_FIFO_MODE mode);
	LMS303_ACCEL_FIFO_MODE getAccelFIFOMode();
	float getAccelX() { return accelX; }
	float getAccelY() { return accelY; }
	float getAccelZ() { return accelZ; }

	imu::Vector<3> read_acc();
	imu::Vector<3> read_mag();

	float getPitch() { return pitch; }
	float getRoll() { return roll; }

	virtual ~LMS303();
};


#endif /* LMS303_H_ */
