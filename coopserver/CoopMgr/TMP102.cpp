//
//  TMP102.cpp
//  coopMgr
//
//  Created by Vincent Moscaritolo on 9/10/21.
//

#include "TMP102.hpp"
#include "LogMgr.hpp"


#define TEMPERATURE_REGISTER 0x00
#define CONFIG_REGISTER 0x01
#define T_LOW_REGISTER 0x02
#define T_HIGH_REGISTER 0x03


TMP102::TMP102(){
	_deviceAddress = 0x00;
}

TMP102::~TMP102(){
	stop();
}

bool TMP102::begin(uint8_t deviceAddress){
	int error = 0;

	return begin(deviceAddress, error);
}
 
bool TMP102::begin(uint8_t deviceAddress,   int &error){
	bool success = false;
	
	_deviceAddress = deviceAddress;
	
	I2C i2cPort;
	if( i2cPort.begin(_deviceAddress, error)) {
		success = true;
		i2cPort.stop();
	}
	return success;
}
 
void TMP102::stop(){
//	LOG_INFO("TMP102(%02x) stop\n",  _i2cPort.getDevAddr());
}
 


uint8_t	TMP102::getDevAddr(){
	return _deviceAddress;
};

bool TMP102::readTempF(float& tempOut){
	
	float cTemp;
	bool status = readTempC(cTemp);
	
	if(status)
		tempOut = cTemp *9.0/5.0 + 32.0;
	
	return status;
};


bool TMP102::readTempC(float& tempOut){
	
	bool success = false;
	
	I2C i2cPort;
	
	if( i2cPort.begin(_deviceAddress)) {
		uint8_t registerByte[2] = {0,0};
		
		if(i2cPort.readBytes(TEMPERATURE_REGISTER, registerByte, 2) == 2) {
			
			int16_t digitalTemp;
			// Bit 0 of second byte will always be 0 in 12-bit readings and 1 in 13-bit
			if(registerByte[1]&0x01)	// 13 bit mode
			{
				// Combine bytes to create a signed int
				digitalTemp = ((registerByte[0]) << 5) | (registerByte[1] >> 3);
				// Temperature data can be + or -, if it should be negative,
				// convert 13 bit to 16 bit and use the 2s compliment.
				if(digitalTemp > 0xFFF)
				{
					digitalTemp |= 0xE000;
				}
			}
			else	// 12 bit mode
			{
				// Combine bytes to create a signed int
				digitalTemp = ((registerByte[0]) << 4) | (registerByte[1] >> 4);
				// Temperature data can be + or -, if it should be negative,
				// convert 12 bit to 16 bit and use the 2s compliment.
				if(digitalTemp > 0x7FF)
				{
					digitalTemp |= 0xF000;
				}
			}
			// Convert digital reading to analog temperature (1-bit is equal to 0.0625 C)
			
			tempOut = digitalTemp*0.0625;
			success = true;
		}
		
		i2cPort.stop();
	}
	
	return success;
}

