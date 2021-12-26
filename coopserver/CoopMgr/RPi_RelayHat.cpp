//
//  RPi_RelayHat.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/23/21.
//

#include "RPi_RelayHat.hpp"
 
RPi_RelayHat::RPi_RelayHat(){
	_isSetup = false;
 }


RPi_RelayHat::~RPi_RelayHat(){
	stop();
	
}
 
bool RPi_RelayHat::begin(string	path){
	int error = 0;

	return begin(path, error);
}

 
bool RPi_RelayHat::begin(string	path,  int &error){
		
	_isSetup = _gpio.begin(path,
								  {CH1,CH2, CH3} ,
								  GPIOD_LINE_REQUEST_DIRECTION_OUTPUT ,
								  {true,true,true} ,error);
	
	return _isSetup;
}


void RPi_RelayHat::stop(){
	
	if(_isSetup){
		_gpio.stop();
	}
	
	_isSetup = false;
}

bool RPi_RelayHat::isAvailable(){

	if(!_isSetup)
		return false;
		
	return true;
}

bool RPi_RelayHat::setRelays(relayStates_t states){
	
	if(!_isSetup)
		return false;
	
	GPIO::gpioStates_t gs;
	
	
	for(const auto& [relay, state] : states) {
		
		// the RPi_RelayHat runs negative logic..  invert the relay
		gs.push_back( make_pair(relay, !state));
//		printf("Set Relay(%2d), %s\n", relay, state?"ON":"OFF");
	}
	if(gs.size() > 0){
		return _gpio.setRelays(gs);
	}
	 
	return false;

}
