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

#define MAX_BUS					64

#define REG_WHO_AM_I			0x0F
#define REG_CTRL0				0x1F
#define REG_BANDWIDTH			0x20
#define REG_STATUS				0x27
#define REG_OUT_X_L_A			0x28
#define REG_OUT_X_H_A			0x29
#define REG_OUT_Y_L_A			0x2A
#define REG_OUT_Y_H_A			0x2B
#define REG_OUT_Z_L_A			0x2C
#define REG_OUT_Z_H_A			0x2D
#define REG_FIFO_CTRL			0x2E
#define REG_FIFO_SRC			0x2F

LMS303Accelerometer::LMS303Accelerometer(int bus, int address) {
	I2CBus = bus;
	I2CAddress = address;
	reset();	// Reset device to default settings
	writeI2CDeviceByte(0x23, 0x00);	// Set accelerometer to SCALE mode
	setAccelBandwidth(BW_100HZ);	// Set bandwidth to enable device.
	setAccelFIFOMode(FIFO_STREAM);
	readFullSensorState();

	accelX = 0;
	accelY = 0;
	accelZ = 0;

	pitch = 0;
	roll = 0;
}

int LMS303Accelerometer::reset() {
	cout << "Resetting LMS303 accelerometer..." << endl;
	writeI2CDeviceByte(REG_CTRL0, 0x80);
	writeI2CDeviceByte(REG_FIFO_SRC, 0x00);	// Set FIFO mode to Bypass
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
	if(this->AccelFIFOMode == FIFO_STREAM) {	// Average the accel measurements stored in FIFO
		int slotsRead = readAccelFIFO(this->AccelFIFO);	// Read Accel FIFO
		averageAccelFIFO(slotsRead);

		// Read the rest of the memory excluding the Accel output registers (because they will burst
		// FIFO data and ruin the burst sequence for the entire memory map.
		readI2CDevice(0x0F, &dataBuffer[REG_WHO_AM_I], REG_STATUS-REG_WHO_AM_I);
		readI2CDevice(0x0F, &dataBuffer[REG_FIFO_CTRL], LMS303_I2C_BUFFER-REG_FIFO_CTRL);
	}
	else {	// No accel output averaging
		readI2CDevice(0x0F, &dataBuffer[0x0F], LMS303_I2C_BUFFER-REG_WHO_AM_I);
	}

	// Check WHO_AM_I register, to make sure I am reading from the registers I think I am.
	if (this->dataBuffer[REG_WHO_AM_I]!=0x49){
		cout << "MAJOR FAILURE: DATA WITH LMS303 HAS LOST SYNC!\t" << endl;
		this->accelX = convertAcceleration(REG_OUT_X_H_A, REG_OUT_X_L_A);
		this->accelY = convertAcceleration(REG_OUT_Y_H_A, REG_OUT_Y_L_A);
		this->accelZ = convertAcceleration(REG_OUT_Z_H_A, REG_OUT_Z_L_A);
	}

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
		cout << "Failure to update bandwidth value!" << endl;
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

int LMS303Accelerometer::setAccelFIFOMode(LMS303_ACCEL_MODE mode) {
	char val[1] = {0x00};
	readI2CDevice(REG_CTRL0, val, 1);	// Read current CTRL0 register


	switch (mode) {
	case FIFO_BYPASS: val[0] = 0x00;		// Leave FIFO mode bits as 0x00 for bypass mode
		break;
	case FIFO_STREAM: {
		val[0] = 0x40;
		writeI2CDeviceByte(REG_CTRL0, (val[0] | 0x40) );	// Enable FIFO bit in CTRL0 register
		break;
	}
	default: val[0] = 0x00;	// Same as bypass mode
		break;
	}
	writeI2CDeviceByte(REG_FIFO_CTRL, val[0]);

	if(getAccelFIFOMode() != mode) cout << "Error setting LMS303 Accelerometer mode!" << endl;
	else this->AccelFIFOMode = mode;
	return 0;
}

LMS303_ACCEL_MODE LMS303Accelerometer::getAccelFIFOMode() {
	char val[1] = { 0x00 };
	readI2CDevice(REG_FIFO_CTRL, val, 1);	// Read current FIFO mode

	switch (val[0]) {
	case 0x00: return FIFO_BYPASS;
		break;
	case 0x40: return FIFO_STREAM;
		break;
	default: return FIFO_ERROR;
		break;
	}

	return FIFO_ERROR;
}

int LMS303Accelerometer::readAccelFIFO(char buffer[]) {
	char val[1] = { 0x00 };
	readI2CDevice(REG_FIFO_SRC, val, 1);	// Read current FIFO mode
	val[0] &= 0x0F;	// Mask all but FIFO slot count bits

	readI2CDevice(REG_OUT_X_L_A, this->AccelFIFO, FIFO_SIZE);	// Read current FIFO mode

	return (int)val[0]+1;	// Return the number of FIFO slots that held new data
}

int LMS303Accelerometer::averageAccelFIFO(int slots){
	int sumX = 0;
	int sumY = 0;
	int sumZ = 0;

	for(int i=0; i<slots; i++) {
		// Reset temp variables
		short tempX = 0x0000;
		short tempY = 0x0000;
		short tempZ = 0x0000;

		// Convert 2's compliment for X, Y & Z
		tempX = this->AccelFIFO[(i*6)+1];
		tempX = (tempX << 8) | this->AccelFIFO[i*6];
		tempX = ~tempX + 1;

		tempY = this->AccelFIFO[(i*6)+3];
		tempY = (tempY << 8) | this->AccelFIFO[(i*6)+2];
		tempY = ~tempY + 1;

		tempZ = this->AccelFIFO[(i*6)+5];
		tempZ = (tempZ << 8) | this->AccelFIFO[(i*6)+4];
		tempZ = ~tempZ + 1;


		// Sum X, Y and Z outputs
		sumX += (int)tempX;
		sumY += (int)tempY;
		sumZ += (int)tempZ;
	}

	this->accelX = sumX / slots;
	this->accelY = sumY / slots;
	this->accelZ = sumZ / slots;

	return 0;
}

LMS303Accelerometer::~LMS303Accelerometer() {
	// TODO Auto-generated destructor stub
}
