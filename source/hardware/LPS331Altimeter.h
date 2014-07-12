/*
 * LPS331Altimeter.h
 *	For use with LPS331 altitude sensor as found in AltIMU-10. Note, must set dataRate to enable
 *	measurements.
 *
 *  Created on: Jul 11, 2014
 *      Author: John Boyd
 *
 *  Reference:
 *  	http://www.inmotion.pt/store/altimu10-v3-gyro-accelerometer-compass-and-altimeter-l3gd20h
 */

#ifndef LPS331Altimeter_H_
#define LPS331Altimeter_H_

#define LPS331_I2C_BUFFER	0x31	// There are 0x31 registers on this device

enum LPS331_ALT_DATA_RATE {
	DR_ALT_ONE_SHOT		= 0,
	DR_ALT_1HZ			= 1,
	DR_ALT_7HZ			= 2,
	DR_ALT_12p5HZ		= 3,
	DR_ALT_25HZ			= 4
};

class LPS331Altimeter {

private:

	int I2CBus, I2CAddress;
	char dataBuffer[LPS331_I2C_BUFFER];

	float pressure;	// in milliBar
	float altitude;	// in meters

	int writeI2CDeviceByte(char address, char value);
	int readI2CDevice(char address, char data[], int size);

	float convertPressure(int msb_reg_addr, int lsb_reg_addr, int Xlsb_reg_addr);
	float convertAltitude(float pressure_mbar);

public:

	LPS331Altimeter(int bus, int address);

	int reset();
	int enableAltimeter();
	int setAltDataRate(LPS331_ALT_DATA_RATE dataRate);
	int readFullSensorState();

	float getPressure() { return pressure; }
	float getAltitude() { return altitude; }

	virtual ~LPS331Altimeter();
};


#endif /* LPS331_H_ */
