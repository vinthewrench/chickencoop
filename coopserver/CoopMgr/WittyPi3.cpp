//
//  WittyPi3.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 1/4/22.
//

#include "WittyPi3.hpp"
#include "LogMgr.hpp"

enum WittyPi3_Register
{
	ID  = 			0x00,			//firmware id
	VOLTAGE_IN_I = 0x01,			//integer part for input voltage
	VOLTAGE_IN_D = 0x02,			//decimal part (x100) for input voltage
	VOLTAGE_OUT_I = 0x03,		//integer part for output voltage
	VOLTAGE_OUT_D = 0x04,		//decimal part (x100) for output voltage
	CURRENT_OUT_I = 0x05,		//integer part for output current
	CURRENT_OUT_D = 0x06,		//decimal part (x100) for output current
	POWER_MODE =	 0x07,		//1 if Witty Pi is powered via the LDO, 0 if direclty use 5V input
	LV_SHUTDOWN 	= 0x08,		//1 if system was shutdown by low voltage, otherwise 0
	
	CONF_ADDRESS 			= 0x09,		//I2C slave address: defaul=0x69
	CONF_DEFAULT_ON 		= 0x10,		//turn on RPi when power is connected: 1=yes, 0=no
	CONF_PULSE_INTERVAL 	= 0x11,		//pulse interval (for LED and dummy load): 9=8s,8=4s,7=2s,6=1s
	CONF_LOW_VOLTAGE 		= 0x12,		//low voltage threshold (x10), 255=disabled
	CONF_BLINK_LED 		= 0x13,		//0 if white LED should not blink. The bigger value, the longer time to light up LED
	CONF_POWER_CUT_DELAY = 0x14,		//the delay (x10) before power cut: default=50 (5 sec)
	CONF_RECOVERY_VOLTAGE = 0x15,		//voltage (x10) that triggers recovery, 255=disabled
	CONF_DUMMY_LOAD 		= 0x16,		//0 if dummy load is off. The bigger value, the longer time to draw current
	CONF_ADJ_VIN 			= 0x17,		//adjustment for measured Vin (x100), range from -127 to 127
	CONF_ADJ_VOUT 			= 0x18,		//adjustment for measured Vout (x100), range from -127 to 127
	CONF_ADJ_IOUT 			= 0x19 		//adjustment for measured Iout (x100), range from -127 to 127
	
	
};

enum DS3231
 {
	CLOCK_TEMP =		0x11,
	
	  TEMP_MSB   = 			0x11, 	//MSB of Temp
	  TEMP_LSB   = 			0x12		//LSB of Temp
  };

WittyPi3::WittyPi3(){
	_state = INS_UNKNOWN;
	_lastQueryTime = {0,0};
	_resultMap.clear();
}

WittyPi3::~WittyPi3(){
	stop();
}

constexpr uint8_t kWpAddr = 0x69;
constexpr uint8_t kClockAddr = 0x68;
 
bool WittyPi3::begin(){
	int error = 0;

	return begin(error);
}

 
bool WittyPi3::begin(int &error){
	
	_state = INS_INVALID;
	_isSetup = false;
 
	if( _i2cWp.begin(kWpAddr, error)
		&& _i2cClock.begin(kClockAddr, error))
	{
		_isSetup = true;
		_state = INS_IDLE;
		_queryDelay = 2;	// seconds
		_lastQueryTime = {0,0};
		_resultMap.clear();
	}
 
	return _isSetup;
}

void WittyPi3::stop(){
	_isSetup = false;
	_state = INS_INVALID;
	_i2cWp.stop();
	_i2cClock.stop();
}

bool WittyPi3::isConnected(){
	return  (_state ==  INS_IDLE || _state ==  INS_RESPONSE);
}
 
 
void WittyPi3::reset(){
	
}

bool WittyPi3::tempC(double &val){

	bool success = false;
 
	union
	{
	  uint8_t  bytes[2] ;
	  uint16_t word ;
	} data;
 
	double digitalTemp;
	
	if(_i2cClock.isAvailable() && _i2cClock.readWord(CLOCK_TEMP, data.word)){
		
		digitalTemp = ((double) (data.bytes[0] & 0x7F)) + ((data.bytes[1] >> 6) * 0.25);
		if((data.bytes[0] & 0x80)  == 0x80) digitalTemp = digitalTemp * -1;
//		printf("WittyPi3::tempC %04x %2.2f\n",data.word, digitalTemp);
		val = digitalTemp;
		success = true;

 	}

	return success;
}

bool WittyPi3::voltageIn(double &val){

	bool success = false;
	uint8_t registerBytes[2] = {0,0};

	// the atmega on WittyPi3 can only handle byte reads
	if( _i2cWp.isAvailable()
		&& _i2cWp.readByte(VOLTAGE_IN_I,registerBytes[0])
		&& _i2cWp.readByte(VOLTAGE_IN_D,registerBytes[1]))
	{
		val = ((double)registerBytes[0]) + (registerBytes[1] * 0.01);
//		  printf("WittyPi3::voltageIn (%02x %02x) %2.2f\n",registerBytes[0],registerBytes[1] , val);
		  success = true;
	}
 
	return success;
}

bool WittyPi3::voltageOut(double &val){
	
	bool success = false;
	uint8_t registerBytes[2] = {0,0};

	// the atmega on WittyPi3 can only handle byte reads
	if( _i2cWp.isAvailable()
		&& _i2cWp.readByte(VOLTAGE_OUT_I,registerBytes[0])
		&& _i2cWp.readByte(VOLTAGE_OUT_D,registerBytes[1]))
	{
		val = ((double)registerBytes[0]) + (registerBytes[1] * 0.01);
//		printf("WittyPi3::voltageOut (%02x %02x) %2.2f\n",registerBytes[0],registerBytes[1] , val);
 		  success = true;
	}
 
	return success;
 }

bool WittyPi3::currentOut(double &val){

	bool success = false;
	uint8_t registerBytes[2] = {0,0};

	// the atmega on WittyPi3 can only handle byte reads
	if( _i2cWp.isAvailable()
		&& _i2cWp.readByte(CURRENT_OUT_I,registerBytes[0])
		&& _i2cWp.readByte(CURRENT_OUT_D,registerBytes[1]))
	{
		val = ((double)registerBytes[0]) + (registerBytes[1] * 0.01);
//		printf("WittyPi3::currentOut (%02x %02x) %2.2f\n",registerBytes[0],registerBytes[1] , val);
		  success = true;
	}
 
	return success;
}

bool WittyPi3::powerMode(bool &val){
 
	uint8_t mode = 0;
	
	bool success = _i2cWp.readByte(POWER_MODE, mode);
	if(success) val = mode == 1;
	
	return success;
}



CoopMgrDevice::response_result_t
WittyPi3::rcvResponse(std::function<void(map<string,string>)> cb){

	CoopMgrDevice::response_result_t result = NOTHING;
	
	if(!isConnected()) {
		return ERROR;
	}
	
	if(_state == INS_RESPONSE){
		result = PROCESS_VALUES;
		if(cb) (cb)(_resultMap);
		_state = INS_IDLE;
	}
	
	
done:
	
	if(result == CONTINUE)
		return result;

	if(result ==  INVALID){
		uint8_t sav =  LogMgr::shared()->_logFlags;
		START_VERBOSE;
		LogMgr::shared()->logTimedStampString("TempSensor INVALID: ");
		LogMgr::shared()->_logFlags = sav;
		return result;
	}
	
	return result;
}

 

CoopMgrDevice::device_state_t WittyPi3::getDeviceState(){
  
  device_state_t retval = DEVICE_STATE_UNKNOWN;
  
  if(!isConnected())
	  retval = DEVICE_STATE_DISCONNECTED;
  
  else if(_state == INS_INVALID)
	  retval = DEVICE_STATE_ERROR;
  
  else retval = DEVICE_STATE_CONNECTED;

  return retval;
}

void WittyPi3::idle(){
	
	if(isConnected() && (_state == INS_IDLE)){
		
		bool shouldQuery = false;
		
		if(_lastQueryTime.tv_sec == 0 &&  _lastQueryTime.tv_usec == 0 ){
			shouldQuery = true;
		} else {
			
			timeval now, diff;
			gettimeofday(&now, NULL);
			timersub(&now, &_lastQueryTime, &diff);
			
			if(diff.tv_sec >=  _queryDelay  ) {
				shouldQuery = true;
			}
		}
		
		if(shouldQuery){
			
//			float tempC;
			
//			if( _sensor.readTempC(tempC)){
//				_resultMap[_resultKey] =  to_string(tempC);
//				_state = INS_RESPONSE;
//				gettimeofday(&_lastQueryTime, NULL);
//
//			}
//			else
//			{
////				_state = INS_INVALID;
//			}
			
		}
	}
}
