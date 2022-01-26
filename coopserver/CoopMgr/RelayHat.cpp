//
//  RelayHat.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 1/26/22.
//

#include "RelayHat.hpp"
 
RelayHat::RelayHat(){
	_isSetup = false;
 }


RelayHat::~RelayHat(){
	stop();
	
}
 
bool RelayHat::begin(string	path){
	int error = 0;

	return begin(path, error);
}

 
bool RelayHat::begin(string	path,  int &error){
		
	// the RelayHat runs positive logic..

	_isSetup = _gpio.begin(path,
								  {CH1,CH2, CH3} ,
								  GPIOD_LINE_REQUEST_DIRECTION_OUTPUT ,
								  0,
								  {false,false,false} ,error);
	
	return _isSetup;
}


void RelayHat::stop(){
	
	if(_isSetup){
		_gpio.stop();
	}
	
	_isSetup = false;
}

bool RelayHat::isAvailable(){

	if(!_isSetup)
		return false;
		
	return true;
}

bool RelayHat::setRelays(relayStates_t states){
	
	if(!_isSetup)
		return false;
	
	GPIO::gpioStates_t gs;
	
	for(const auto& [relay, state] : states) {
		gs.push_back( make_pair(relay,  state));
//		printf("Set Relay(%2d), %s\n", relay, state?"ON":"OFF");
	}
	if(gs.size() > 0){
		return _gpio.setRelays(gs);
	}
	 
	return false;

}

bool RelayHat::getRelays(relayStates_t &states){
	
	if(!_isSetup)
		return false;
	
	GPIO::gpioStates_t gs;

	if( _gpio.getRelays(gs)){
		
		for(const auto& [relay, state] : gs) {
			relaysID_t relayID = (relaysID_t)relay;
			states.push_back( make_pair(relayID,  state));
		}
		return true;
	}
	
	return false;
 }
 
