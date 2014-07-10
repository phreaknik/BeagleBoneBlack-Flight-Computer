/*
 * LMS303.cpp
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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stropts.h>
#include <stdio.h>
#include "LMS303.h"
#include <iostream>
#include <math.h>
using namespace std;

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


LMS303::LMS303(int bus, int address) {
	I2CBus = bus;
	I2CAddress = address;
	reset();	// Reset device to default settings
	writeI2CDeviceByte(0x23, 0x00);	// Set accelerometer to SCALE mode
	setAccelBandwidth(BW_100HZ);	// Set bandwidth to enable device.
	setAccelFIFOMode(FIFO_STREAM);
	enableTempSensor();
	readFullSensorState();

	accelX = 0;
	accelY = 0;
	accelZ = 0;

	pitch = 0;
	roll = 0;
}

int LMS303::reset() {
	cout << "Resetting LMS303 accelerometer..." << endl;
	writeI2CDeviceByte(REG_CTRL0, 0x80);
	writeI2CDeviceByte(REG_FIFO_SRC, 0x00);	// Set FIFO mode to Bypass
	memset(dataBuffer, 0, LMS303_I2C_BUFFER);
	sleep(1);
	cout << "Done." << endl;
	return 0;
}

int LMS303::readFullSensorState() {
	/* Since this device is actually multiple sensors from different companies manufactured on
	 * one piece of silicon, the I2C communication blocks for each sensor are not identical.
	 * As such, a block read across both the beginning magnetometer registers and the accelerometer
	 * registers causes the devices to glitch and give corrupt data. The line below is a
	 * simple block read command, but with the addresses offset so that the block read doesn't
	 * try to read from any of the magnetometer addresses (registers 0x00-0x0E).
	 */
	if(this->AccelFIFOMode == FIFO_STREAM) {	// Average the accel measurements stored in FIFO
		// Read the rest of the memory excluding the Accel output registers (because they will burst
		// FIFO data and ruin the burst sequence for the entire memory map.


		readI2CDevice(REG_TEMP_OUT_L, &dataBuffer[REG_TEMP_OUT_L], (REG_OUT_Z_H_M-REG_TEMP_OUT_L)+1);
		readI2CDevice(REG_WHO_AM_I, &dataBuffer[REG_WHO_AM_I], 1);
		readI2CDevice(REG_INT_CTRL_M, &dataBuffer[REG_INT_CTRL_M], (REG_STATUS_A-REG_INT_CTRL_M)+1);
		readI2CDevice(REG_FIFO_CTRL, &dataBuffer[REG_FIFO_CTRL], LMS303_I2C_BUFFER-REG_FIFO_CTRL);

		// Read accel FIFO afterwards to prevent I2C glitch
		int slotsRead = readAccelFIFO(this->AccelFIFO);	// Read Accel FIFO
		averageAccelFIFO(slotsRead);
	}
	else {	// No accel output averaging
		readI2CDevice(0x0F, &dataBuffer[0x0F], LMS303_I2C_BUFFER-REG_WHO_AM_I);
	}

	// Check WHO_AM_I register, to make sure I am reading from the registers I think I am.
	if (dataBuffer[REG_WHO_AM_I]!=0x49){
		cout << "MAJOR FAILURE: DATA WITH LMS303 HAS LOST SYNC!\t" << endl;
		this->accelX = convertAcceleration(REG_OUT_X_H_A, REG_OUT_X_L_A);
		this->accelY = convertAcceleration(REG_OUT_Y_H_A, REG_OUT_Y_L_A);
		this->accelZ = convertAcceleration(REG_OUT_Z_H_A, REG_OUT_Z_L_A);
	}

	readTemperature();
	calculatePitchAndRoll();

	return(0);
}

int LMS303::enableTempSensor() {
	char buf[1] = {0x00};
	readI2CDevice(REG_CTRL5, buf, 1);	// Read current register state
	buf[0] |= 0x80;	// Set TEMP_EN bit
	writeI2CDeviceByte(REG_CTRL5, buf[0]);	// Write back to register

	// Check if bit is set
	readI2CDevice(REG_CTRL5, buf, 1);
	buf[0] &= 0x80;
	if(buf[0] == 0x80) return 0;
	else {
		cout << "ERROR: Failed to enable temperature sensor.\n";
		return 1;
	}
}

int LMS303::readTemperature() {
	// Read temperature registers directly into dataBuffer
	// Datasheet is not clear, so temp conversion may be inaccurate.
	// Not verified with negative temperatures;

	short temp = dataBuffer[REG_TEMP_OUT_H];
	temp = (temp << 8) | (dataBuffer[REG_TEMP_OUT_L]);

	// Mask MSBs appropriately to convert 12 bit 2s compliment to 16 bit
	if(temp & 0x0800) temp |= 0x8000;
	else temp &= 0x0FFF;

	this->celsius = (int)temp;
	cout << "Temp:\t" << std::dec << this->celsius << "C\n";

	return 0;
}

void LMS303::calculatePitchAndRoll() {
	double accelXSquared = this->accelX * this->accelX;
	double accelYSquared = this->accelY * this->accelY;
	double accelZSquared = this->accelZ * this->accelZ;
	this->pitch = 180 * atan(accelX/sqrt(accelYSquared + accelZSquared))/M_PI;
	this->roll = 180 * atan(accelY/sqrt(accelXSquared + accelZSquared))/M_PI;
}

int LMS303::convertAcceleration(int msb_reg_addr, int lsb_reg_addr){
	short temp = dataBuffer[msb_reg_addr];
	temp = (temp<<8) | dataBuffer[lsb_reg_addr];
	return (int)temp;
}

int LMS303::setAccelBandwidth(LMS303_ACCEL_BANDWIDTH bandwidth){
	char temp = bandwidth << 4;	//move value into bits 7,6,5,4
	temp += 0b00000111; //enable X,Y,Z axis bits

	if(writeI2CDeviceByte(REG_CTRL1, temp)!=0){
		cout << "Failure to update bandwidth value!" << endl;
		return 1;
	}
	return 0;
}

LMS303_ACCEL_BANDWIDTH LMS303::getAccelBandwidth(){

	char buf[1];
	if(readI2CDevice(REG_CTRL1, buf, 1)!=0){
		cout << "Failure to read bandwidth value" << endl;
		return BW_ERROR;
	}

	return (LMS303_ACCEL_BANDWIDTH)buf[0];
}

int LMS303::writeI2CDeviceByte(char address, char value) {
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

int LMS303::readI2CDevice(char address, char data[], int size){
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

    char temp = address;
    if(size > 1) temp |= 0b10000000;	// Set MSB to enable burst if reading multiple bytes.
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

int LMS303::setAccelFIFOMode(LMS303_ACCEL_MODE mode) {
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

LMS303_ACCEL_MODE LMS303::getAccelFIFOMode() {
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

int LMS303::readAccelFIFO(char buffer[]) {
	char val[1] = { 0x00 };
	readI2CDevice(REG_FIFO_SRC, val, 1);	// Read current FIFO mode
	val[0] &= 0x0F;	// Mask all but FIFO slot count bits

	readI2CDevice(REG_OUT_X_L_A, this->AccelFIFO, FIFO_SIZE);	// Read current FIFO mode

	return (int)val[0]+1;	// Return the number of FIFO slots that held new data
}

int LMS303::averageAccelFIFO(int slots){
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

LMS303::~LMS303() {
	// TODO Auto-generated destructor stub
}
