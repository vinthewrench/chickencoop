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

CoopDevices::CoopDevices(){
	_state = INS_UNKNOWN;
}

CoopDevices::~CoopDevices(){
	stop();
}

 
bool CoopDevices::begin(int &error){
 
	if(! _relay.begin("/dev/gpiochip0", error )){
		return false;
	}
	

	return true;
}

void CoopDevices::stop(){
 
}

bool CoopDevices::isConnected(){
	return  false;
}
 
void CoopDevices::reset(){
	
}

void CoopDevices::idle(){
	
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
bool CoopDevices::setDoor(bool isOpen, boolCallback_t cb){
	
	bool didSucceed = false;
 
	if(!_relay.isAvailable())
		return false;
	
	if(isOpen){
		didSucceed = _relay.setRelays({{RPi_RelayHat::CH2, true}, {RPi_RelayHat::CH3, false}});
	}
	else
	{
		didSucceed = _relay.setRelays({{RPi_RelayHat::CH2, false}, {RPi_RelayHat::CH3, true}});
	}
	
	if(cb) (cb)(didSucceed);
	return  true;

}
 
CoopDevices::door_state_t CoopDevices::getDoorState(){
	return  STATE_UNKNOWN;
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

bool CoopDevices::getLight(){
	
	return  false;

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
