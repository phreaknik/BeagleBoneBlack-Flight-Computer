/*
 * L3GD20Gyro.h
 *	For use with L3GD20 gyroscope as found in AltIMU-10. Note, must set dataRate to enable
 *	measurements.
 *
 *  Created on: Jul 11, 2014
 *      Author: John Boyd
 *
 *  Reference:
 *  	http://www.inmotion.pt/store/altimu10-v3-gyro-accelerometer-compass-and-altimeter-l3gd20h
 */

#ifndef L3GD20Gyro_H_
#define L3GD20Gyro_H_

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

#define L3GD20_I2C_BUFFER	0x40	// There are 0x31 registers on this device
#define GYRO_FIFO_SLOTS			0x20	// Number of slots in fifo for each axis
#define GYRO_FIFO_SIZE			0xC0	// 32 slots * 6 FIFO registers
#define MAX_BUS						64

#define REG_WHO_AM_I				0x0F
#define REG_CTRL1					0x20
#define REG_CTRL2					0x21
#define REG_CTRL3					0x22
#define REG_CTRL4					0x23
#define REG_CTRL5					0x24
#define REG_REFERENCE				0x25
#define REG_OUT_TEMP				0x26
#define REG_STATUS					0x27
#define REG_OUT_X_L					0x28
#define REG_OUT_X_H					0x29
#define REG_OUT_Y_L					0x2A
#define REG_OUT_Y_H					0x2B
#define REG_OUT_Z_L					0x2C
#define REG_OUT_Z_H					0x2D
#define REG_FIFO_CTRL				0x2E
#define REG_FIFO_SRC				0x2F
#define REG_IG_CFG					0x30
#define REG_IG_SRC					0x31
#define REG_IG_THS_XH				0x32
#define REG_IG_THS_XL				0x33
#define REG_IG_THS_YH				0x34
#define REG_IG_THS_YL				0x35
#define REG_IG_THS_ZH				0x36
#define REG_IG_THS_ZL				0x37
#define REG_IG_DURATION				0x38
#define REG_LOW_ODR					0x39

enum L3GD20_GYRO_SCALE {
	SCALE_GYRO_245dps		= 0,
	SCALE_GYRO_500dps		= 1,
	SCALE_GYRO_2000dps		= 2
};

enum L3GD20_DATA_RATE {
	DR_GYRO_100HZ			= 0,
	DR_GYRO_200HZ			= 1,
	DR_GYRO_400HZ			= 2,
	DR_GYRO_800HZ			= 3
};

enum L3GD20_GYRO_FIFO_MODE {
	GYRO_FIFO_BYPASS,
	GYRO_FIFO_STREAM,	// This mode enables averaging
	GYRO_FIFO_ERROR
};

class L3GD20Gyro {

private:

	int I2CBus, I2CAddress;
	char dataBuffer[L3GD20_I2C_BUFFER];
	char gyroFIFO[GYRO_FIFO_SIZE];
	L3GD20_GYRO_FIFO_MODE gyroFIFOMode = GYRO_FIFO_BYPASS;

	float gyroScale;
	float gyroX;
	float gyroY;
	float gyroZ;

	int writeI2CDeviceByte(char address, char value);
	int readI2CDevice(char address, char data[], int size);
	int readGyroFIFO(char buffer[]);
	int averageGyroFIFO(int slots);
	float convertGyroOutput(int msb_reg_addr, int lsb_reg_addr);	// Convert output to degrees per second
	float convertGyroOutput(int rate);	// Convert output to degrees per second

public:

	L3GD20Gyro(int bus, int address);

	int reset();
	int enableGyro();
	int setGyroDataRate(L3GD20_DATA_RATE dataRate);
	int setGyroScale(L3GD20_GYRO_SCALE scale);
	int setGyroFIFOMode(L3GD20_GYRO_FIFO_MODE mode);
	int readFullSensorState();

	float getGyroX() { return gyroX; }
	float getGyroY() { return gyroY; }
	float getGyroZ() { return gyroZ; }

	imu::Vector<3> read_gyro();

	virtual ~L3GD20Gyro();
};


#endif /* L3GD20_H_ */
