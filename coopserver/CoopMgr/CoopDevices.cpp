//
//  CoopDevices.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/21/21.
//

#include "CoopDevices.hpp"
#include "LogMgr.hpp"
#include "Utils.hpp"
#include <regex>
#include <ctype.h>

CoopDevices::CoopDevices():
						_doorMgr(this)
{
	_state = INS_UNKNOWN;
	_lastQueryTime = {0,0};
	_resultMap.clear();
}

CoopDevices::~CoopDevices(){
	stop();
}

 
bool CoopDevices::begin(int &error){
  
	int  errnum = 0;

	  _state = INS_IDLE;
	  _queryDelay = 2;	// seconds
	  _lastQueryTime = {0,0};

	if(! _relay.begin("/dev/gpiochip0", error )){
		return false;
	}
	
 	if( _redButton.begin(0x6E, errnum))
		LOGT_DEBUG("Start RedButton - OK");
 	else
		LOGT_ERROR("Start RedButton  - FAIL %s", string(strerror(errnum)).c_str());
	
	
//	if(_redButton.isOpen()){
//
//		uint8_t  deviceType;
//		uint16_t  version;
//		if(_redButton.getDeviceType(deviceType)){
//
//			printf("_redButton.getDeviceType() -> %02x\n", deviceType);
//		}
//
//		if(_redButton.getFirmwareVersion(version)){
//			printf("_redButton.getFirmwareVersion() -> %04x\n", version);
//		}
//
//	}
	
	if( _greenButton.begin(0x6F, errnum))
		LOGT_DEBUG("Start GreenButton - OK");
	else
		LOGT_ERROR("Start GreenButton  - FAIL %s", string(strerror(errnum)).c_str());

//	if(_greenButton.isOpen()){
//
//		uint8_t  deviceType;
//		uint16_t  version;
//		if(_greenButton.getDeviceType(deviceType)){
//
//			printf("_greenButton.getDeviceType() -> %02x\n", deviceType);
//		}
//
//		if(_greenButton.getFirmwareVersion(version)){
//			printf("_greenButton.getFirmwareVersion() -> %04x\n", version);
//		}
//
//	}


	_doorMgr.begin();

	return true;
}

void CoopDevices::stop(){
 
	_state = INS_INVALID;

	if(_relay.isAvailable()){
		_relay.stop();
	}
}

bool CoopDevices::isConnected(){
	return  _relay.isAvailable();
}
 
void CoopDevices::reset(){
	
}

void CoopDevices::idle(){
	
 	if(_state == INS_IDLE){
		
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
		
			this->getLight([=] (bool didSucceed,bool isOn ){
				if(didSucceed){
					_resultMap[string(COOP_DEVICE_LIGHT_STATE)] =  to_string(isOn);
					_state = INS_RESPONSE;
				}
			});

			this->getDoor([=] (bool didSucceed, door_state_t state ){
				if(didSucceed){
					_resultMap[string(COOP_DEVICE_DOOR_STATE)] =  to_string(state);
					_state = INS_RESPONSE;
				}
			});
			gettimeofday(&_lastQueryTime, NULL);
		}
	}
	_doorMgr.idle();
}

CoopMgrDevice::response_result_t
CoopDevices::rcvResponse(std::function<void(map<string,string>)> cb){

	CoopMgrDevice::response_result_t result = NOTHING;
	
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
		LogMgr::shared()->logTimedStampString("CPUInfo INVALID: ");
		LogMgr::shared()->_logFlags = sav;
		return result;
	}
	return result;
}



CoopMgrDevice::device_state_t CoopDevices::getDeviceState(){
  
  device_state_t retval = DEVICE_STATE_UNKNOWN;
  
  if(!isConnected())
	  retval = DEVICE_STATE_DISCONNECTED;
  
  else if(_state == INS_INVALID)
	  retval = DEVICE_STATE_ERROR;
  
  else retval = DEVICE_STATE_CONNECTED;

  return retval;
}

// door state

bool CoopDevices::doorOpen(boolCallback_t cb){
	if(!_relay.isAvailable())
		return false;
	
	_doorMgr.startOpen();
	if(cb) (cb)(true);

	return true;
}

bool CoopDevices::doorClose(boolCallback_t cb){
	if(!_relay.isAvailable())
		return false;
	
	_doorMgr.startClose();
	if(cb) (cb)(true);

	return true;
}

bool CoopDevices::doorStop(boolCallback_t cb){
	if(!_relay.isAvailable())
		return false;

	_doorMgr.stop();
	if(cb) (cb)(true);

	return true;
}


bool CoopDevices::setDoor(bool isOpen, boolCallback_t cb){
	
	bool didSucceed = false;
 
	if(!_relay.isAvailable())
		return false;
	
	if(isOpen){
		_greenButton.LEDconfig(0x1f, 500, 100);
		_redButton.LEDoff();
		didSucceed = _relay.setRelays({{RPi_RelayHat::CH2, true}, {RPi_RelayHat::CH3, false}});
	}
	else
	{
		_redButton.LEDconfig(0x1f, 500, 100);
		_greenButton.LEDoff();
		didSucceed = _relay.setRelays({{RPi_RelayHat::CH2, false}, {RPi_RelayHat::CH3, true}});
	}
	
	if(cb) (cb)(didSucceed);
	return  true;
}

bool CoopDevices::stopDoor(boolCallback_t cb){
	
	bool didSucceed = false;
	
	_redButton.LEDoff();
	_greenButton.LEDoff();

	if(!_relay.isAvailable())
		return false;
	
	didSucceed = _relay.setRelays({{RPi_RelayHat::CH2, false}, {RPi_RelayHat::CH3, false}});
	
	if(cb) (cb)(didSucceed);
	return  true;
}

 
bool CoopDevices::getDoor(std::function<void(bool didSucceed, door_state_t state)> cb){
	
	if(!_relay.isAvailable())
		  return false;

	if(cb) (cb)(true, STATE_UNKNOWN);
	return  true;

}

// light state
bool CoopDevices::setLight(bool isOn, boolCallback_t cb){
	bool didSucceed = false;

	if(!_relay.isAvailable())
		return false;
 
	didSucceed = _relay.setRelays({{RPi_RelayHat::CH1, isOn}});

	if(cb) (cb)(didSucceed);
	return  true;
}


bool CoopDevices::getLight(std::function<void(bool didSucceed, bool isOn)> cb){
	
	if(!_relay.isAvailable())
		return false;
	
	RPi_RelayHat::relayStates_t  states;
	
	if(_relay.getRelays(states)){
		for(const auto& [relay, state] : states) {
			
			if(relay == RPi_RelayHat::CH1) {
				if(cb) (cb)(true, state);
				return true;
			}
		}
	}
	
	if(cb) (cb)(false, false);
	return false;
}

 
bool CoopDevices::stringToRelayState(const std::string str, bool* stateOut){
	bool valid = false;

	if(!empty(str)){
		bool state = false;
		const char * param1 = str.c_str();
		int intValue = atoi(param1);

		// check for level
		if(std::regex_match(param1, std::regex("^[0-1]$"))){
			state = bool(intValue);
			valid = true;
		}
		else {
			if(caseInSensStringCompare(str,"off")) {
				state = false;
				valid = true;
			}
			else if(caseInSensStringCompare(str,"on")) {
				state = true;
				valid = true;
			}
			else if(caseInSensStringCompare(str,"open")) {
				state = true;
				valid = true;
			}
			else if(caseInSensStringCompare(str,"close")) {
				state = false;
				valid = true;
			}
		}

		if(valid && stateOut)
			*stateOut = state;
	}
	return valid;
}
bool CoopDevices::jsonToRelayState( nlohmann::json j, bool* stateOut){

	bool state = false;
	bool isValid = false;

	if( j.is_string()){
		string str = j;
		if(CoopDevices::stringToRelayState(str, &state)){
			isValid = true;
		}
	}
	else if( j.is_number()){
		int num = j;
		if(num == 0 || num == 1){
			state = bool(num);
			isValid = true;
		}
	}
	else if( j.is_boolean()){
		bool val = j;
		state =  val;
		isValid = true;
	}

	if(stateOut)
		*stateOut = state;

	return isValid;
}
