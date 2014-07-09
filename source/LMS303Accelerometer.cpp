/*
 * LMS303Accelerometer.cpp
 *	For use with LMS303 Accelerometer as found in AltIMU-10. Note, must set bandwidth to enable
 *	measurements.
 *
 *  Created on: Jul 3, 2014
 *      Author: phreaknux
 *
 *  Reference:
 *  	http://www.inmotion.pt/store/altimu10-v3-gyro-accelerometer-compass-and-altimeter-l3gd20h
 *  	http://inmotion.pt/documentation/pololu/POL-2469/LSM303D.pdf
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stropts.h>
#include <stdio.h>
#include "LMS303Accelerometer.h"
#include <iostream>
#include <math.h>
using namespace std;

#define MAX_BUS	64

#define REG_WHO_AM_I			0x0F
#define REG_CTRL0				0x1F
#define REG_BANDWIDTH			0x20
#define REG_STATUS				0x27
#define REG_OUT_X_Low			0x28
#define REG_OUT_X_High			0x29
#define REG_OUT_Y_Low			0x2A
#define REG_OUT_Y_High			0x2B
#define REG_OUT_Z_Low			0x2C
#define REG_OUT_Z_High			0x2D
#define REG_FIFO_CTRL			0x2E
#define REG_FIFO_SRC			0x2F

LMS303Accelerometer::LMS303Accelerometer(int bus, int address) {
	I2CBus = bus;
	I2CAddress = address;
	reset();	// Reset device to default settings
	writeI2CDeviceByte(0x23, 0x00);	// Set accelerometer to SCALE mode
	setAccelBandwidth(BW_200HZ);	// Set bandwidth to enable device.
	readFullSensorState();

	// Delete these if no bugs arise
	//char buf[0x40] = { 0x00 };
	//readI2CDevice(0x0F, &buf[0x0F], 0x40-0x0F);

	accelX = 0;
	accelY = 0;
	accelZ = 0;

	pitch = 0;
	roll = 0;
}

int LMS303Accelerometer::reset() {
	cout << "Resetting LMS303 accelerometer..." << endl;
	writeI2CDeviceByte(REG_CTRL0, 0x80);
	sleep(1);
	cout << "Done." << endl;
	return 0;
}

void LMS303Accelerometer::calculatePitchAndRoll() {
	double accelXSquared = this->accelX * this->accelX;
	double accelYSquared = this->accelY * this->accelY;
	double accelZSquared = this->accelZ * this->accelZ;
	this->pitch = 180 * atan(accelX/sqrt(accelYSquared + accelZSquared))/M_PI;
	this->roll = 180 * atan(accelY/sqrt(accelXSquared + accelZSquared))/M_PI;
}

int LMS303Accelerometer::readFullSensorState() {
	/* Since this device is actually multiple sensors from different companies manufactured on
	 * one piece of silicon, the I2C communication blocks for each sensor are not identical.
	 * As such, a block read across both the beginning magnetometer registers and the accelerometer
	 * registers causes the devices to glitch and give corrupt data. The line below is a
	 * simple block read command, but with the addresses offset so that the block read doesn't
	 * try to read from any of the magnetometer addresses (registers 0x00-0x0E).
	 */
	readI2CDevice(0x0F, &dataBuffer[0x0F], LMS303_I2C_BUFFER-0x0F);

	// Check WHO_AM_I register, to make sure I am reading from the registers I think I am.
	if (this->dataBuffer[REG_WHO_AM_I]!=0x49){
	    	cout << "MAJOR FAILURE: DATA WITH LMS303 HAS LOST SYNC!\t" << endl;
	}

	this->accelX = convertAcceleration(REG_OUT_X_High, REG_OUT_X_Low);
	this->accelY = convertAcceleration(REG_OUT_Y_High, REG_OUT_Y_Low);
	this->accelZ = convertAcceleration(REG_OUT_Z_High, REG_OUT_Z_Low);
	calculatePitchAndRoll();

	return(0);
}

int LMS303Accelerometer::convertAcceleration(int msb_reg_addr, int lsb_reg_addr){
	short temp = dataBuffer[msb_reg_addr];
	temp = (temp<<8) | dataBuffer[lsb_reg_addr];
	temp = ~temp + 1;
	return (int)temp;
}

int LMS303Accelerometer::setAccelBandwidth(LMS303_ACCEL_BANDWIDTH bandwidth){
	char temp = bandwidth << 4;	//move value into bits 7,6,5,4
	temp += 0b00000111; //enable X,Y,Z axis bits

	if(writeI2CDeviceByte(REG_BANDWIDTH, temp)!=0){
		cout << "Failure to update bandwidth value" << endl;
		return 1;
	}
	return 0;
}

LMS303_ACCEL_BANDWIDTH LMS303Accelerometer::getAccelBandwidth(){

	char buf[1];
	if(readI2CDevice(REG_BANDWIDTH, buf, 1)!=0){
		cout << "Failure to read bandwidth value" << endl;
		return BW_ERROR;
	}

	return (LMS303_ACCEL_BANDWIDTH)buf[0];
}

int LMS303Accelerometer::writeI2CDeviceByte(char address, char value) {
	char namebuf[MAX_BUS];
	snprintf(namebuf, sizeof(namebuf), "/dev/i2c-%d", I2CBus);
	int file;
	if ((file = open(namebuf, O_RDWR)) < 0){
		cout << "Failed to open LMS303 Sensor on " << namebuf << " I2C Bus" << endl;
		return(1);
	}
	if (ioctl(file, I2C_SLAVE, I2CAddress) < 0){
		cout << "I2C_SLAVE address " << I2CAddress << " failed..." << endl;
		return(2);
	}

	char buffer[2];
	buffer[0] = address;
	buffer[1] = value;
	if ( write(file, buffer, 2) != 2) {
		cout << "Failure to write values to I2C Device address." << endl;
		return(3);
	}
	close(file);
	return 0;
}

int LMS303Accelerometer::readI2CDevice(char address, char data[], int size){
    char namebuf[MAX_BUS];
   	snprintf(namebuf, sizeof(namebuf), "/dev/i2c-%d", 1);
    int file;
    if ((file = open(namebuf, O_RDWR)) < 0){
            cout << "Failed to open LMS303 Sensor on " << namebuf << " I2C Bus" << endl;
            return(1);
    }
    if (ioctl(file, I2C_SLAVE, 0x1d) < 0){
            cout << "I2C_SLAVE address " << 0x1d << " failed..." << endl;
            return(2);
    }

    // According to the LMS303 datasheet on page 22, to read from the device, we must first
    // send the address to read from in write mode. The MSB of the address must be 1 to enable "block reading"
    // then we may read as many bytes as desired.

    char temp = address | 0b10000000; // Set MSB to enable reading multiple bytes.
    char buf[1] = { temp };
    if(write(file, buf, 1) !=1){
    	cout << "Failed to set address to read from in readFullSensorState() " << endl;
    }

    if ( read(file, data, size) != size) {
        cout << "Failure to read value from I2C Device address." << endl;
    }

    close(file);
    return 0;
}

int LMS303Accelerometer::setAccelMode(LMS303_ACCEL_MODE mode) {
	char val[1] = {0x00};
	readI2CDevice(REG_CTRL0, val, 1);	// Read current CTRL0 register
	writeI2CDeviceByte(REG_CTRL0, (val[0] | 0x40) );	// Enable FIFO bit in CTRL0 register

	switch (mode) {
	case FIFO_BYPASS: val[0] = 0x00;		// Leave FIFO mode bits as 0x00 for bypass mode
		break;
	case FIFO_STREAM: val[0] = 0x40;
		break;
	default: val[0] = 0x00;	// Same as bypass mode
		break;
	}
	writeI2CDeviceByte(REG_FIFO_CTRL, val[0]);

	return 0;
}

LMS303_ACCEL_MODE LMS303Accelerometer::getAccelMode() {
	char val[1] = { 0x00 };
	readI2CDevice(REG_FIFO_CTRL, val, 1);	// Read current FIFO mode

	switch (val[1]) {
	case 0b00000000: return FIFO_BYPASS;
		break;
	case 0b01000000: return FIFO_STREAM;
		break;
	default: return FIFO_ERROR;
		break;
	}

	return FIFO_ERROR;
}

int LMS303Accelerometer::getAccelFIFOAverage(char buffer[], int size) {
	char FIFO[0x0F] = {0x00};
	readI2CDevice(REG_FIFO_SRC, FIFO, 1);	// Check how many FIFO slots are available to be read.
	FIFO[0] &= 0x0F;	// Mask all but FIFO status bits
	//readI2CDevice(REG_)
	return 0;
}
LMS303Accelerometer::~LMS303Accelerometer() {
	// TODO Auto-generated destructor stub
}
