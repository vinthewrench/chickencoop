//
//  GPIO.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 12/21/21.
//

#include "GPIO.hpp"


GPIO::GPIO(){
	_isSetup = false;
 }


GPIO::~GPIO(){
	stop();
	
}
 
bool GPIO::begin(string	path){
	int error = 0;

	return begin(path, &error);
}


bool GPIO::begin(string	path,   int * errorOut){
 	_isSetup = false;
	return _isSetup;
}


void GPIO::stop(){
	
	if(_isSetup){
	}
	
	_isSetup = false;
}

bool GPIO::isAvailable(){

	if(!_isSetup)
		return false;
		
	return true;
}
