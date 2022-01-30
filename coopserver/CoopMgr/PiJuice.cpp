//
//  PiJuice.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 1/29/22.
//

#include "PiJuice.hpp"
#include "LogMgr.hpp"
#include "Utils.hpp"

enum PiJuice_cmd : uint8_t
{
	STATUS_CMD = 0x40,
	FAULT_EVENT_CMD = 0x44,
	CHARGE_LEVEL_CMD = 0x41,
	BUTTON_EVENT_CMD = 0x45,
	BATTERY_TEMPERATURE_CMD = 0x47,
	BATTERY_VOLTAGE_CMD = 0x49,
	BATTERY_CURRENT_CMD = 0x4b,
	IO_VOLTAGE_CMD = 0x4d,
	IO_CURRENT_CMD = 0x4f,
	LED_STATE_CMD = 0x66,
	LED_BLINK_CMD = 0x68,
	IO_PIN_ACCESS_CMD = 0x75,
};


PiJuice::PiJuice(){
	_state = INS_UNKNOWN;
	_lastQueryTime = {0,0};
	_resultMap.clear();
}

PiJuice::~PiJuice(){
	stop();
}

constexpr uint8_t kPJAddr = 0x14;
 
bool PiJuice::begin(){
	int error = 0;

	return begin(error);
}

 
bool PiJuice::begin(int &error){
	
	_state = INS_INVALID;
	_isSetup = false;
 
	if( _i2cPJ.begin(kPJAddr, error) )
	{
		_isSetup = true;
		_state = INS_IDLE;
		_queryDelay = 2;	// seconds
		_lastQueryTime = {0,0};
		_resultMap.clear();
	}
 
	return _isSetup;
}

void PiJuice::stop(){
	_isSetup = false;
	_state = INS_INVALID;
	_i2cPJ.stop();
 }

bool PiJuice::isConnected(){
	return  (_state ==  INS_IDLE || _state ==  INS_RESPONSE);
}
 
 

void PiJuice::reset(){
  
}


CoopMgrDevice::response_result_t
PiJuice::rcvResponse(std::function<void(map<string,string>)> cb){

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

 
bool PiJuice::SOC(double &val){ 	// state of charge
	bool success = false;
	
	I2C::i2c_block_t block;
	
	if( _i2cPJ.isAvailable()
		&& _i2cPJ.readBlock(PiJuice_cmd::CHARGE_LEVEL_CMD,1, block) ){
		
		val = (double)  block[1];
		success = true;
	}
	
	return success;
}

bool PiJuice::batteryVoltage(double &val){
	bool success = false;
	
	I2C::i2c_block_t block;
	
	if( _i2cPJ.isAvailable()
		&& _i2cPJ.readBlock(PiJuice_cmd::BATTERY_VOLTAGE_CMD,2, block) ){
	 
		val =  double((block[2] << 8) | block[1])/1000.0 ;
		success = true;
	}
	
	return success;
}


bool PiJuice::currentOut(double &val){
	bool success = false;
	
	I2C::i2c_block_t block;
	
	if( _i2cPJ.isAvailable()
		&& _i2cPJ.readBlock(PiJuice_cmd::IO_CURRENT_CMD,2, block) ){
	 
		uint16_t i = (block[2] << 8) | block[1];
		if (i & (1 << 15))  i = i - (1 << 16);

		val =  double(i)/1000.0;
		success = true;
	}
	
	return success;
}

bool  PiJuice::tempC(double &val){
	bool success = false;
	
	I2C::i2c_block_t block;
	
	if( _i2cPJ.isAvailable()
		&& _i2cPJ.readBlock(PiJuice_cmd::BATTERY_TEMPERATURE_CMD,2, block) ){
		
		uint8_t temp = block[1];
		if (block[1] & (1 << 7))  temp = temp - (1 << 8);
 
		val =  double(temp);
		success = true;
	}
	
	return success;
}

bool PiJuice::status(piStatus_t &status,  piFault_t &fault){
	bool success = false;
	I2C::i2c_block_t statusBlock;
	I2C::i2c_block_t faultBlock;

	if( _i2cPJ.isAvailable()
		&& _i2cPJ.readBlock(PiJuice_cmd::STATUS_CMD,1, statusBlock)
		&&  _i2cPJ.readBlock(PiJuice_cmd::FAULT_EVENT_CMD,1, faultBlock)){
	 
		status = {.byteWrapped = statusBlock[1]};
		fault = {.byteWrapped = faultBlock[1]};
	
 		success = true;
	}

	return success;
}
 

CoopMgrDevice::device_state_t PiJuice::getDeviceState(){
  
  device_state_t retval = DEVICE_STATE_UNKNOWN;
  
  if(!isConnected())
	  retval = DEVICE_STATE_DISCONNECTED;
  
  else if(_state == INS_INVALID)
	  retval = DEVICE_STATE_ERROR;
  
  else retval = DEVICE_STATE_CONNECTED;

  return retval;
}

void PiJuice::idle(){
	
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
			
			bool hasValue = false;
			
			double dbl;
//			bool bl;

			if( tempC(dbl)){
				_resultMap["PJ_TEMP"] =  to_string(dbl);
				hasValue = true;
			}

			if( SOC(dbl)){
				_resultMap["PJ_SOC"] =  to_string(dbl);
				hasValue = true;
		}

			if( currentOut(dbl)){
				_resultMap["PJ_I"] =  to_string(dbl);
				hasValue = true;
			}

			if( batteryVoltage(dbl)){
				_resultMap["PJ_BV"] =  to_string(dbl);
				hasValue = true;
		}
			
			piStatus_t  pi_status;
			piFault_t   pi_fault;
			
			if( status(pi_status, pi_fault)){
				_resultMap["PJ_STATUS"] =  to_hex<unsigned char> (pi_status.byteWrapped);
				_resultMap["PJ_FAULT"] =  to_hex<unsigned char> (pi_fault.byteWrapped);
				hasValue = true;
		}
	
		 
//			if( powerMode(bl)){
//				_resultMap["PWR_MODE"] =  to_string(bl);
//				hasValue = true;
//			}

			if(hasValue){
				_state = INS_RESPONSE;
				gettimeofday(&_lastQueryTime, NULL);
			}
		}
	}
}
