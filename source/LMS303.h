/*
 * LMS303Accelerometer.h
 *	For use with LMS303 Accelerometer as found in AltIMU-10. Note, must set bandwidth to enable
 *	measurements.
 *
 *  Created on: Jul 3, 2014
 *      Author: phreaknux
 *
 *  Reference:
 *  	http://www.inmotion.pt/store/altimu10-v3-gyro-accelerometer-compass-and-altimeter-l3gd20h
 */

#ifndef LMS303ACCELEROMETER_H_
#define LMS303ACCELEROMETER_H_

#define LMS303_I2C_BUFFER		0x40	// Only 0x40 registers available according to LMS303 datasheet
#define FIFO_SLOTS				0x0F	// Number of slots in FIFO for each accelerometer output
#define FIFO_SIZE 				0x60	// Size of FIFO array

enum LMS303_ACCEL_BANDWIDTH {
	BW_SHUTDOWN	= 0,
	BW_3p125HZ 	= 1,
	BW_6p25HZ 	= 2,
	BW_12p5HZ 	= 3,
	BW_25HZ 	= 4,
	BW_50HZ 	= 5,
	BW_100HZ 	= 6,
	BW_200HZ 	= 7,
	BW_4OOHZ 	= 8,
	BW_8OOHZ 	= 9,
	BW_16OOHZ 	= 10,
	BW_ERROR	= -1
};

enum LMS303_ACCEL_MODE {
	FIFO_BYPASS,
	FIFO_STREAM,	// This mode enables averaging
	FIFO_ERROR
};

class LMS303Accelerometer {

private:
	int I2CBus, I2CAddress;
	char dataBuffer[LMS303_I2C_BUFFER];
	char AccelFIFO[FIFO_SIZE];	// 16 FIFO slots * 6 Accel output registers
	LMS303_ACCEL_MODE AccelFIFOMode = FIFO_BYPASS;

	int accelX;
	int accelY;
	int accelZ;

	double pitch;	// in degrees
	double roll;	// in degrees

	int convertAcceleration(int msb_reg_addr, int lsb_reg_addr);
	int writeI2CDeviceByte(char address, char value);
	int readI2CDevice(char address, char data[], int size);
	void calculatePitchAndRoll();
	int readAccelFIFO(char buffer[]);
	int averageAccelFIFO(int slots);

public:
	LMS303Accelerometer(int bus, int address);

	int reset();
	int readFullSensorState();
	int setAccelBandwidth(LMS303_ACCEL_BANDWIDTH bandwidth);	// Must set bandwidth to enable device
	LMS303_ACCEL_BANDWIDTH getAccelBandwidth();
	int setAccelFIFOMode(LMS303_ACCEL_MODE mode);
	LMS303_ACCEL_MODE getAccelFIFOMode();

	int getAccelX() { return accelX; }
	int getAccelY() { return accelY; }
	int getAccelZ() { return accelZ; }

	float getPitch() { return pitch; }
	float getRoll() { return roll; }

	virtual ~LMS303Accelerometer();
};


#endif /* LMS303ACCELEROMETER_H_ */
