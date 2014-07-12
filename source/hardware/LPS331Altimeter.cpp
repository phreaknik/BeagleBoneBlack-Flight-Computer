/*
 * LPS331Altimeter.cpp
 *	For use with LPS331 altitude sensor as found in AltIMU-10. Note, must set dataRate to enable
 *	measurements.
 *
 *  Created on: Jul 11, 2014
 *      Author: John Boyd
 *
 *  Reference:
 *  	http://www.inmotion.pt/store/altimu10-v3-gyro-accelerometer-compass-and-altimeter-l3gd20h
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
#include "LPS331Altimeter.h"
#include <iostream>
#include <math.h>
using namespace std;


#define MAX_BUS						64

#define	REG_REF_P_XL				0x08
#define REG_REF_P_L					0x09
#define REG_REF_P_H					0x0A
#define REG_WHO_AM_I				0x0F
#define REG_RES_CONF				0x10
#define REG_CTRL_REG1				0x20
#define REG_CTRL_REG2				0x21
#define REG_CTRL_REG3				0x22
#define REG_INT_CFG_REG				0x23
#define REG_INT_SOURCE_REG			0x24
#define REG_THS_P_LOW_REG			0x25
#define REG_THS_P_HIGH_REG			0x26
#define REG_STATUS_REG				0x27
#define REG_PRESS_POUT_XL_REH		0x28
#define REG_PRESS_OUT_L				0x29
#define REG_PRESS_OUT_H				0x2A
#define REG_TEMP_OUT_L				0x2B
#define REG_TEMP_OUT_H				0x2C
#define REG_AMP_CTRL				0x30

LPS331Altimeter::LPS331Altimeter(int bus, int address) {
	I2CBus = bus;
	I2CAddress = address;

	pressure = 0;
	altitude = 0;

	reset();	// Reset device to default settings
	enableAltimeter();
	readFullSensorState();
}

int LPS331Altimeter::reset() {
	cout << "Resetting LPS331 altimeter...\t";

	// Reset device
	writeI2CDeviceByte(REG_CTRL_REG2, 0x80);	// Reboot device

	// Clear memory
	memset(dataBuffer, 0, LPS331_I2C_BUFFER);
	pressure = 0;
	altitude = 0;

	sleep(1);
	cout << "Done." << endl;
	return 0;
}

int LPS331Altimeter::enableAltimeter() {
	char buf[1];
	readI2CDevice(REG_CTRL_REG1, buf, 1);	// Read current value
	buf[0] |= 0x80;	// Set power down bit (turn on device)
	if(writeI2CDeviceByte(REG_CTRL_REG1, buf[0])) {	// Set accelerometer SCALE
		cout << "Failed to turn on altimiter!" << endl;
		return 1;
	}

	setAltDataRate(DR_ALT_7HZ);
	return 0;
}

int LPS331Altimeter::readFullSensorState() {
	// Read registers into memory
	readI2CDevice(REG_REF_P_XL, &dataBuffer[REG_REF_P_XL], (REG_RES_CONF-REG_REF_P_XL)+1);
	readI2CDevice(REG_CTRL_REG1, &dataBuffer[REG_CTRL_REG1], (REG_TEMP_OUT_H-REG_CTRL_REG1)+1);
	readI2CDevice(REG_AMP_CTRL, &dataBuffer[REG_AMP_CTRL], 1);

	// Check WHO_AM_I register, to make sure I am reading from the registers I think I am.
	if (dataBuffer[REG_WHO_AM_I]!=0xBB){
		cout << "MAJOR FAILURE: DATA WITH LPS331 ALTIMETER HAS LOST SYNC!\t" << endl;
		return (1);
	}

	pressure = convertPressure(REG_PRESS_OUT_H, REG_PRESS_OUT_L, REG_PRESS_POUT_XL_REH);	// Conver pressure to mbar
	altitude = convertAltitude(pressure);	// convert mbar to altitude in meters

	//for(int i=0; i< LPS331_I2C_BUFFER; i++) cout << std::hex << i << "\t" << (int)dataBuffer[i] << endl;
	return(0);
}

int LPS331Altimeter::setAltDataRate(LPS331_ALT_DATA_RATE dataRate) {
	char buf[1];
	readI2CDevice(REG_CTRL_REG1, buf, 1);	// Read current value
	buf[0] &= 0x8F;	// Clear ODR bits
	buf[0] |= (char)dataRate << 4;	// Set new scale bits
	if(writeI2CDeviceByte(REG_CTRL_REG1, buf[0])) {	// Set accelerometer SCALE
		cout << "Failed to set altimeter dataRate!" << endl;
		return 1;
	}
	return 0;
}

int LPS331Altimeter::writeI2CDeviceByte(char address, char value) {
	char namebuf[MAX_BUS];
	snprintf(namebuf, sizeof(namebuf), "/dev/i2c-%d", I2CBus);
	int file;
	if ((file = open(namebuf, O_RDWR)) < 0){
		cout << "Failed to open LPS331 altitude sensor on " << namebuf << " I2C Bus" << endl;
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

int LPS331Altimeter::readI2CDevice(char address, char data[], int size){
    char namebuf[MAX_BUS];
   	snprintf(namebuf, sizeof(namebuf), "/dev/i2c-%d", 1);
    int file;
    if ((file = open(namebuf, O_RDWR)) < 0){
            cout << "Failed to open LPS331 altitude sensor on " << namebuf << " I2C Bus" << endl;
            return(1);
    }
    if (ioctl(file, I2C_SLAVE, I2CAddress) < 0){
            cout << "I2C_SLAVE address " << 0x1d << " failed..." << endl;
            return(2);
    }

    // According to the LPS331 datasheet, to read from the device, we must first
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

float LPS331Altimeter::convertPressure(int msb_reg_addr, int lsb_reg_addr, int xlsb_reg_addr) {
	int temp = dataBuffer[msb_reg_addr];
	temp = (temp << 8) | dataBuffer[lsb_reg_addr];
	temp = (temp << 8) | dataBuffer[xlsb_reg_addr];

	return (float)temp / 4096;	// in mBar
}

float LPS331Altimeter::convertAltitude(float pressure_mbar) {
	return (1 - pow(pressure_mbar/1013.25, 0.190263)) * 44330.8;
}

LPS331Altimeter::~LPS331Altimeter() {
	// TODO Auto-generated destructor stub
}
