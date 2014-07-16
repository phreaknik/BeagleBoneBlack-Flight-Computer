/*
 * L3GD20Gyro.cpp
 *	For use with L3GD20 gyroscope as found in AltIMU-10. Note, must set dataRate to enable
 *	measurements.
 *
 *  Created on: Jul 11, 2014
 *      Author: John Boyd
 *
 *  Reference:
 *  	http://www.inmotion.pt/store/altimu10-v3-gyro-accelerometer-compass-and-altimeter-l3gd20h
 */

#include "L3GD20Gyro.h"

using namespace std;

L3GD20Gyro::L3GD20Gyro(int bus, int address) {
	I2CBus = bus;
	I2CAddress = address;

	reset();	// Reset device to default settings
	enableGyro();
	readFullSensorState();
}

int L3GD20Gyro::reset() {
	cout << "Resetting L3DG20 gyroscope...\t\t" << std::flush;

	// Reset device
	writeI2CDeviceByte(REG_CTRL5, 0x80);	// Reboot device
	writeI2CDeviceByte(REG_CTRL1, 0x00);	// Reset Accel settings
	writeI2CDeviceByte(REG_CTRL2, 0x00);	// Reset Accel settings
	writeI2CDeviceByte(REG_CTRL3, 0x00);	// Reset interrupt settings
	writeI2CDeviceByte(REG_CTRL4, 0x00);	// Reset interrupt settings
	writeI2CDeviceByte(REG_FIFO_CTRL, 0x00);	// Set FIFO mode to Bypass
	writeI2CDeviceByte(REG_FIFO_SRC, 0x00);	// Set FIFO mode to Bypass

	// Clear memory
	memset(dataBuffer, 0, L3GD20_I2C_BUFFER);
	memset(gyroFIFO, 0, GYRO_FIFO_SIZE);
	gyroScale = 0;

	gyroFIFOMode = GYRO_FIFO_BYPASS;

	gyroX = 0;
	gyroY = 0;
	gyroZ = 0;

	sleep(1);
	cout << "Done." << endl;
	return 0;
}

int L3GD20Gyro::enableGyro() {
	setGyroDataRate(DR_GYRO_800HZ);	// Set dataRate to enable device.
	setGyroScale(SCALE_GYRO_2000dps);	// Set accelerometer SCALE
	setGyroFIFOMode(GYRO_FIFO_STREAM);	// Enable FIFO for easy output averaging

	char buf[1] = {0x00};
	readI2CDevice(REG_CTRL1, buf, 1);	// Read current register state
	buf[0] |= 0x0F;		// Set power down and X,Y,Z enable bits

	if(writeI2CDeviceByte(REG_CTRL1, buf[0])!=0){
		cout << "Failure to enable gyro!" << endl;
		return 1;
	}
	return 0;
}

int L3GD20Gyro::readFullSensorState() {
	// Read registers into memory
	if(gyroFIFOMode == GYRO_FIFO_STREAM) {	// Average the gyro measurements stored in FIFO
		// Read the rest of the memory excluding the gyro output registers (because they will burst
		// FIFO data and ruin the burst sequence for the entire memory map.
		readI2CDevice(REG_WHO_AM_I, &dataBuffer[REG_WHO_AM_I], 1);
		readI2CDevice(REG_CTRL1, &dataBuffer[REG_CTRL1], (REG_STATUS-REG_CTRL1)+1);
		readI2CDevice(REG_FIFO_CTRL, &dataBuffer[REG_FIFO_CTRL], L3GD20_I2C_BUFFER-REG_FIFO_CTRL);

		// Read gyro FIFO afterwards to prevent I2C glitch
		int slotsRead = readGyroFIFO(gyroFIFO);
		averageGyroFIFO(slotsRead);
	}
	else {	// No accel output averaging
		readI2CDevice(REG_WHO_AM_I, &dataBuffer[REG_WHO_AM_I], L3GD20_I2C_BUFFER-REG_WHO_AM_I);

		gyroX = convertGyroOutput(REG_OUT_X_H, REG_OUT_X_L);	// Convert to degrees per second
		gyroY = convertGyroOutput(REG_OUT_Y_H, REG_OUT_Y_L);	// Convert to degrees per second
		gyroZ = convertGyroOutput(REG_OUT_Z_H, REG_OUT_Z_L);	// Convert to degrees per second
	}

	// Check WHO_AM_I register, to make sure I am reading from the registers I think I am.
	if (dataBuffer[REG_WHO_AM_I]!=0xD7){
		cout << "MAJOR FAILURE: DATA WITH LPS331 ALTIMETER HAS LOST SYNC!\t" << endl;
		return (1);
	}

	//for(int i=0; i< L3GD20_I2C_BUFFER; i++) cout << std::hex << i << "\t" << (int)dataBuffer[i] << endl;
	return(0);
}

int L3GD20Gyro::setGyroDataRate(L3GD20_DATA_RATE dataRate) {
	char buf[1];
	readI2CDevice(REG_CTRL1, buf, 1);	// Read current value
	buf[0] &= 0x3F;	// Clear ODR bits
	buf[0] |= (char)dataRate << 6;	// Set new scale bits
	if(writeI2CDeviceByte(REG_CTRL1, buf[0])) {	// Set accelerometer SCALE
		cout << "Failed to set altimeter dataRate!" << endl;
		return 1;
	}
	return 0;
}

int L3GD20Gyro::writeI2CDeviceByte(char address, char value) {
	char namebuf[MAX_BUS];
	snprintf(namebuf, sizeof(namebuf), "/dev/i2c-%d", I2CBus);
	int file;
	if ((file = open(namebuf, O_RDWR)) < 0){
		cout << "Failed to open L3GD20 gyroscope on " << namebuf << " I2C Bus" << endl;
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

int L3GD20Gyro::readI2CDevice(char address, char data[], int size){
    char namebuf[MAX_BUS];
   	snprintf(namebuf, sizeof(namebuf), "/dev/i2c-%d", 1);
    int file;
    if ((file = open(namebuf, O_RDWR)) < 0){
            cout << "Failed to open L3GD20 gyroscope on " << namebuf << " I2C Bus" << endl;
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


int L3GD20Gyro::setGyroScale(L3GD20_GYRO_SCALE scale) {
	char buf[1];
	readI2CDevice(REG_CTRL4, buf, 1);	// Read current value
	buf[0] &= 0xCF;	// Clear scale bits
	buf[0] |= (char)scale << 4;	// Set new scale bits
	if(writeI2CDeviceByte(REG_CTRL4, buf[0])) {	// Set accelerometer SCALE
		cout << "Failed to set accelerometer scale!" << endl;
		gyroScale = 0;
		return 1;
	}

	switch(scale){
	case SCALE_GYRO_245dps: {
		gyroScale = .00875;
		break;
	}
	case SCALE_GYRO_500dps: {
		gyroScale = .0175;
		break;
	}
	case SCALE_GYRO_2000dps: {
		gyroScale = .07;
		break;
	}
	default: {
		gyroScale = 0;
		cout << "Error! Invalid gyroscope scale." << endl;
		return 1;
		break;
	}
	}
	return 0;
}

int L3GD20Gyro::setGyroFIFOMode(L3GD20_GYRO_FIFO_MODE mode) {
	char val1[1] = {0x00};
	readI2CDevice(REG_CTRL5, val1, 1);	// Read current CTRL0 register

	char val2[1] = {0x00};
	readI2CDevice(REG_FIFO_CTRL, val2, 1);	// Read current CTRL0 register


	switch (mode) {
	case GYRO_FIFO_BYPASS: {
		val1[0] &= 0xBF;		// Clear FIFO enable bit
		val2[0] &= 0x1F;		// Clear FIFO mode bits
		break;
	}
	case GYRO_FIFO_STREAM: {
		val1[0] |= 0x40;		// Enable FIFO
		val2[0] |= 0x40;		// Set FIFO mode bits
		break;
	}
	default: {
		val1[0] &= 0xBF;	// Same as bypass mode
		val2[0] &= 0x1F;		// Clear FIFO mode bits
		break;
	}
	}
	if(writeI2CDeviceByte(REG_CTRL5, (val1[0] | 0x40) )) {	// Enable FIFO bit in CTRL5 register
		cout<< "Failed to set gyroscope FIFO mode!" << endl;
		return 1;
	}
	if(writeI2CDeviceByte(REG_FIFO_CTRL, val2[0])) {
		cout<< "Failed to set gyroscope FIFO mode!" << endl;
		return 1;
	}

	gyroFIFOMode = mode;
	return 0;
}

float L3GD20Gyro::convertGyroOutput(int msb_reg_addr, int lsb_reg_addr){
	short temp = dataBuffer[msb_reg_addr];
	temp = (temp<<8) | dataBuffer[lsb_reg_addr];
	return ((float)temp * gyroScale);	// Convert to dps
}

float L3GD20Gyro::convertGyroOutput(int rate) {
	return ((float)rate * gyroScale);	// Convert to g's
}

int L3GD20Gyro::readGyroFIFO(char buffer[]) {
	char val[1] = { 0x00 };
	readI2CDevice(REG_FIFO_SRC, val, 1);	// Read current FIFO mode

	if(val[0] & 0x20) {
		cout << "Failed to read gyro FIFO, because FIFO is empty!" << endl;
		return 1;
	}
	val[0] &= 0x1F;	// Mask all but FIFO slot count bits

	readI2CDevice(REG_OUT_X_L, gyroFIFO, GYRO_FIFO_SIZE);	// Read current FIFO mode

	return (int)val[0]+1;	// Return the number of FIFO slots that held new data
}

int L3GD20Gyro::averageGyroFIFO(int slots) {
	if(slots <= 0) {
		cout << "Error! Divide by 0 in averageGyroFIFO()!" << endl;
		return 1;
	}

	int sumX = 0;
	int sumY = 0;
	int sumZ = 0;

	for(int i=0; i<slots; i++) {
		// Reset temp variables
		short tempX = 0x0000;
		short tempY = 0x0000;
		short tempZ = 0x0000;

		// Convert 2's compliment for X, Y & Z
		tempX = gyroFIFO[(i*6)+1];
		tempX = (tempX << 8) | gyroFIFO[i*6];
		tempX = ~tempX + 1;

		tempY = gyroFIFO[(i*6)+3];
		tempY = (tempY << 8) | gyroFIFO[(i*6)+2];
		tempY = ~tempY + 1;

		tempZ = gyroFIFO[(i*6)+5];
		tempZ = (tempZ << 8) | gyroFIFO[(i*6)+4];
		tempZ = ~tempZ + 1;

		// Sum X, Y and Z outputs
		sumX += (int)tempX;
		sumY += (int)tempY;
		sumZ += (int)tempZ;
	}

	gyroX = convertGyroOutput(sumX / slots);
	gyroY = convertGyroOutput(sumY / slots);
	gyroZ = convertGyroOutput(sumZ / slots);

	return 0;
}

imu::Vector<3> L3GD20Gyro::read_gyro() {
	imu::Vector<3> gyro(gyroX,gyroY,gyroZ);
	return gyro;
}

L3GD20Gyro::~L3GD20Gyro() {
	// TODO Auto-generated destructor stub
}
