/*
 * LMS303.cpp
 *	For use with LMS303 Accelerometer as found in AltIMU-10. Note, must set dataRate to enable
 *	measurements.
 *
 *  Created on: Jul 3, 2014
 *      Author: John Boyd
 *
 *  Reference:
 *  	http://www.inmotion.pt/store/altimu10-v3-gyro-accelerometer-compass-and-altimeter-l3gd20h
 *  	http://inmotion.pt/documentation/pololu/POL-2469/LSM303D.pdf
 */

#include "LMS303.h"

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

	accelX = 0;
	accelY = 0;
	accelZ = 0;

	pitch = 0;
	roll = 0;

	reset();	// Reset device to default settings
	enableMagnetometer();
	enableAccelerometer();
	enableTempSensor();
	readFullSensorState();
}

int LMS303::reset() {
	cout << "Resetting LMS303 accelerometer...\t" << std::flush;
	// Reset control registers
	writeI2CDeviceByte(REG_CTRL0, 0x80);	// Reboot LMS303 memory
	writeI2CDeviceByte(REG_CTRL1, 0x00);	// Reset Accel settings
	writeI2CDeviceByte(REG_CTRL2, 0x00);	// Reset Accel settings
	writeI2CDeviceByte(REG_CTRL3, 0x00);	// Reset interrupt settings
	writeI2CDeviceByte(REG_CTRL4, 0x00);	// Reset interrupt settings
	writeI2CDeviceByte(REG_CTRL5, 0x00);	// Reset TEMP/MAG settings
	writeI2CDeviceByte(REG_CTRL6, 0x00);	// Reset MAG settings
	writeI2CDeviceByte(REG_CTRL7, 0x00);	// Reset TEMP/MAG/ACCEL settings
	writeI2CDeviceByte(REG_FIFO_CTRL, 0x00);	// Set FIFO mode to Bypass
	writeI2CDeviceByte(REG_FIFO_SRC, 0x00);	// Set FIFO mode to Bypass

	// Clear memory
	memset(dataBuffer, 0, LMS303_I2C_BUFFER);	// Clear dataBuffer
	memset(accelFIFO, 0, ACCEL_FIFO_SIZE);	// Clear accelFIFO

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
	if(accelFIFOMode == ACCEL_FIFO_STREAM) {	// Average the accel measurements stored in FIFO
		// Read the rest of the memory excluding the Accel output registers (because they will burst
		// FIFO data and ruin the burst sequence for the entire memory map.


		readI2CDevice(REG_TEMP_OUT_L, &dataBuffer[REG_TEMP_OUT_L], (REG_OUT_Z_H_M-REG_TEMP_OUT_L)+1);
		readI2CDevice(REG_WHO_AM_I, &dataBuffer[REG_WHO_AM_I], 1);
		readI2CDevice(REG_INT_CTRL_M, &dataBuffer[REG_INT_CTRL_M], (REG_STATUS_A-REG_INT_CTRL_M)+1);
		readI2CDevice(REG_FIFO_CTRL, &dataBuffer[REG_FIFO_CTRL], LMS303_I2C_BUFFER-REG_FIFO_CTRL);

		// Read accel FIFO afterwards to prevent I2C glitch
		int slotsRead = readAccelFIFO(accelFIFO);	// Read Accel FIFO
		averageAccelFIFO(slotsRead);
	}
	else {	// No accel output averaging
		readI2CDevice(REG_WHO_AM_I, &dataBuffer[REG_WHO_AM_I], LMS303_I2C_BUFFER-REG_WHO_AM_I);

		accelX = convertAcceleration(REG_OUT_X_H_A, REG_OUT_X_L_A);
		accelY = convertAcceleration(REG_OUT_Y_H_A, REG_OUT_Y_L_A);
		accelZ = convertAcceleration(REG_OUT_Z_H_A, REG_OUT_Z_L_A);
	}

	// Check WHO_AM_I register, to make sure I am reading from the registers I think I am.
	if (dataBuffer[REG_WHO_AM_I]!=0x49){
		cout << "MAJOR FAILURE: DATA WITH LMS303 HAS LOST SYNC!\t" << endl;
		return (1);
	}

	getTemperature();

	magX = convertMagnetism(REG_OUT_X_H_M, REG_OUT_X_L_M);
	magY = convertMagnetism(REG_OUT_Y_H_M, REG_OUT_Y_L_M);
	magZ = convertMagnetism(REG_OUT_Z_H_M, REG_OUT_Z_L_M);

	calculatePitchAndRoll();

	return(0);
}

int LMS303::enableTempSensor() {
	char buf[1] = {0x00};
	readI2CDevice(REG_CTRL5, buf, 1);	// Read current register state
	buf[0] |= 0x80;	// Set TEMP_EN bit

	if(writeI2CDeviceByte(REG_CTRL5, buf[0])) {	// Write back to register
		cout << "ERROR: Failed to enable temperature sensor.\n";
		return 1;
	}

	return 0;
}

int LMS303::getTemperature() {
	// Read temperature registers directly into dataBuffer
	// Datasheet is not clear, so temp conversion may be inaccurate.
	// Not verified with negative temperatures;

	short temp = dataBuffer[REG_TEMP_OUT_H];
	temp = (temp << 8) | (dataBuffer[REG_TEMP_OUT_L]);

	// Mask MSBs appropriately to convert 12 bit 2s complement to 16 bit 2s complement
	if(temp & 0x0800) temp |= 0x8000;
	else temp &= 0x0FFF;

	celsius = (int)temp;

	return celsius;
}

int LMS303::enableMagnetometer() {
	setMagDataRate(DR_MAG_12p5HZ);	// Set dataRate to enable device.
	setMagScale(SCALE_MAG_2gauss);	// Set accelerometer SCALE

	char buf[1] = {0x00};
	readI2CDevice(REG_CTRL7, buf, 1);	// Read current register state
	buf[0] &= 0xF8;		// Clear low-power bit and mode bits
	if(writeI2CDeviceByte(REG_CTRL7, buf[0])) {
		cout << "Failed to enable magnetometer!" << endl;
		return 1;
	}

	return 0;
}

int LMS303::setMagScale(LMS303_MAG_SCALE scale) {	// Set magnetometer output rate
	char buf = (char)scale << 5;		// Clear low-power bit and mode bits
	if(writeI2CDeviceByte(REG_CTRL6, buf)) {
		cout << "Failed to set magnetometer scale!" << endl;
		magFullScale = 0;
		return 1;
	}

	switch(scale){
	case SCALE_MAG_2gauss: {
		magFullScale = 2;
		break;
	}
	case SCALE_MAG_4gauss: {
		magFullScale = 4;
		break;
	}
	case SCALE_MAG_8gauss: {
		magFullScale = 8;
		break;
	}
	case SCALE_MAG_12gauss: {
		magFullScale = 12;
		break;
	}
	default: {
		magFullScale = 0;
		return 1;
		break;
	}
	}
	return 0;
}

int LMS303::setMagDataRate(LMS303_MAG_DATA_RATE dataRate) {	// Set magnetometer SCALE
	char buf[1] = {0x00};
	readI2CDevice(REG_CTRL5, buf, 1);	// Read current register state
	buf[0] &= 0x82;		// Clear resolution and dataRate bits
	buf[0] |= 0x20;	// Set resolution bits to medium resolution
	buf[0] |= (char)dataRate << 2;	// Set dataRate bits
	if(writeI2CDeviceByte(REG_CTRL5, buf[0])) {	// write back to ctrl register
		cout << "Failed to set magnetometer dataRate!" << endl;
		return 1;
	}
	return 0;
}

float LMS303::convertMagnetism(int msb_reg_addr, int lsb_reg_addr){
	short temp = dataBuffer[msb_reg_addr];
	temp = (temp<<8) | dataBuffer[lsb_reg_addr];
	return ((float)temp * (float)magFullScale) / (float)0x8000;	// Convert to gauss
}

int LMS303::enableAccelerometer() {
	setAccelDataRate(DR_ACCEL_100HZ);	// Set dataRate to enable device.
	setAccelScale(SCALE_ACCEL_2g);	// Set accelerometer SCALE
	setAccelFIFOMode(ACCEL_FIFO_STREAM);	// Enable FIFO for easy output averaging

	char buf[1] = {0x00};
	readI2CDevice(REG_CTRL1, buf, 1);	// Read current register state
	buf[0] |= 0x07;		// Set X,Y,Z enable bits

	if(writeI2CDeviceByte(REG_CTRL1, buf[0])!=0){
			cout << "Failure to enable accelerometer!" << endl;
			return 1;
	}
	return 0;
}

int LMS303::setAccelScale(LMS303_ACCEL_SCALE scale) {
	char buf[1];
	readI2CDevice(REG_CTRL2, buf, 1);	// Read current value
	buf[0] &= 0b11000111;	// Clear scale bits
	buf[0] |= (char)scale << 3;	// Set new scale bits
	if(writeI2CDeviceByte(REG_CTRL2, buf[0])) {	// Set accelerometer SCALE
		cout << "Failed to set accelerometer scale!" << endl;
		accelFullScale = 0;
		return 1;
	}

	switch(scale){
	case SCALE_ACCEL_2g: {
		accelFullScale = 2;
		break;
	}
	case SCALE_ACCEL_4g: {
		accelFullScale = 4;
		break;
	}
	case SCALE_ACCEL_6g: {
		accelFullScale = 6;
		break;
	}
	case SCALE_ACCEL_8g: {
		accelFullScale = 8;
		break;
	}
	case SCALE_ACCEL_16g: {
		accelFullScale = 16;
		break;
	}
	default: {
		accelFullScale = 0;
		return 1;
		break;
	}
	}
	return 0;
}

void LMS303::calculatePitchAndRoll() {
	double accelXSquared = this->accelX * this->accelX;
	double accelYSquared = this->accelY * this->accelY;
	double accelZSquared = this->accelZ * this->accelZ;
	this->pitch = 180 * atan(accelX/sqrt(accelYSquared + accelZSquared))/M_PI;
	this->roll = 180 * atan(accelY/sqrt(accelXSquared + accelZSquared))/M_PI;
}

float LMS303::convertAcceleration(int msb_reg_addr, int lsb_reg_addr){
	short temp = dataBuffer[msb_reg_addr];
	temp = (temp<<8) | dataBuffer[lsb_reg_addr];
	return ((float)temp * (float)accelFullScale) / (float)0x8000;	// Convert to g's
}

float LMS303::convertAcceleration(int accel) {
	return ((float)accel * (float)accelFullScale) / (float)0x8000;	// Convert to g's
}

int LMS303::setAccelDataRate(LMS303_ACCEL_DATA_RATE dataRate){
	char temp = dataRate << 4;	//move value into bits 7,6,5,4

	if(writeI2CDeviceByte(REG_CTRL1, temp)!=0){
		cout << "Failure to update dataRate value!" << endl;
		return 1;
	}
	return 0;
}

LMS303_ACCEL_DATA_RATE LMS303::getAccelDataRate(){

	char buf[1];
	if(readI2CDevice(REG_CTRL1, buf, 1)!=0){
		cout << "Failure to read dataRate value" << endl;
		return DR_ACCEL_ERROR;
	}

	return (LMS303_ACCEL_DATA_RATE)buf[0];
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
    if (ioctl(file, I2C_SLAVE, I2CAddress) < 0){
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

int LMS303::setAccelFIFOMode(LMS303_ACCEL_FIFO_MODE mode) {
	char val[1] = {0x00};
	readI2CDevice(REG_CTRL0, val, 1);	// Read current CTRL0 register


	switch (mode) {
	case ACCEL_FIFO_BYPASS: {
		val[0] = 0x00;		// Leave FIFO mode bits as 0x00 for bypass mode
		break;
	}
	case ACCEL_FIFO_STREAM: {
		val[0] = 0x40;
		writeI2CDeviceByte(REG_CTRL0, (val[0] | 0x40) );	// Enable FIFO bit in CTRL0 register
		break;
	}
	default: {
		val[0] = 0x00;	// Same as bypass mode
		break;
	}
	}
	writeI2CDeviceByte(REG_FIFO_CTRL, val[0]);

	if(getAccelFIFOMode() != mode) cout << "Error setting LMS303 Accelerometer mode!" << endl;
	else accelFIFOMode = mode;
	return 0;
}

LMS303_ACCEL_FIFO_MODE LMS303::getAccelFIFOMode() {
	char val[1] = { 0x00 };
	readI2CDevice(REG_FIFO_CTRL, val, 1);	// Read current FIFO mode

	switch (val[0]) {
	case 0x00: return ACCEL_FIFO_BYPASS;
		break;
	case 0x40: return ACCEL_FIFO_STREAM;
		break;
	default: return ACCEL_FIFO_ERROR;
		break;
	}

	return ACCEL_FIFO_ERROR;
}

int LMS303::readAccelFIFO(char buffer[]) {
	char val[1] = { 0x00 };
	readI2CDevice(REG_FIFO_SRC, val, 1);	// Read current FIFO mode
	val[0] &= 0x0F;	// Mask all but FIFO slot count bits

	readI2CDevice(REG_OUT_X_L_A, accelFIFO, ACCEL_FIFO_SIZE);	// Read current FIFO mode

	return (int)val[0]+1;	// Return the number of FIFO slots that held new data
}

int LMS303::averageAccelFIFO(int slots){
	if(slots <= 0) {
		cout << "Error! Divide by 0 in averageAccelFIFO()!" << endl;
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
		tempX = this->accelFIFO[(i*6)+1];
		tempX = (tempX << 8) | this->accelFIFO[i*6];
		tempX = ~tempX + 1;

		tempY = this->accelFIFO[(i*6)+3];
		tempY = (tempY << 8) | this->accelFIFO[(i*6)+2];
		tempY = ~tempY + 1;

		tempZ = this->accelFIFO[(i*6)+5];
		tempZ = (tempZ << 8) | this->accelFIFO[(i*6)+4];
		tempZ = ~tempZ + 1;


		// Sum X, Y and Z outputs
		sumX += (int)tempX;
		sumY += (int)tempY;
		sumZ += (int)tempZ;
	}

	accelX = convertAcceleration(sumX / slots);
	accelY = convertAcceleration(sumY / slots);
	accelZ = convertAcceleration(sumZ / slots);

	return 0;
}

imu::Vector<3> LMS303::read_acc() {
	imu::Vector<3> acc(accelX, accelY, accelZ);
	return acc;
}

imu::Vector<3> LMS303::read_mag() {
	imu::Vector<3> mag(magX,magY,magZ);
	return mag;
}

LMS303::~LMS303() {
	// TODO Auto-generated destructor stub
}
