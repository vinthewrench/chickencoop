//
//  GPIO.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 12/21/21.
//

#include "GPIO.hpp"


#if !defined(__APPLE__)
#include <gpiod.h>
#endif

#include "LogMgr.hpp"

GPIO::GPIO(){
	_isSetup = false;
 }


GPIO::~GPIO(){
	stop();
	
}
 
bool GPIO::begin(string	path, vector<uint8_t> lines){
	int error = 0;

	return begin(path, lines, error);
}


bool GPIO::begin(string	path, vector<uint8_t> pins,  int &error){
 
	struct gpiod_line_request_config config;

	int initial_values[3] = {0,0,0};

	// Create an array of size equivalent to vector
	 unsigned int offsets[ pins.size() ];
	  // Copy all elements of vector to array
	  std::transform( pins.begin(),  pins.end(),  offsets, [](const auto & elem){  return elem; });
   
// debug
	for(auto x: offsets) { 	printf("Setup GPIO(%2d)\n", x); }
//

	// open the device
	_chip = gpiod_chip_open(path.c_str());
	if(!_chip) {
		LOGT_ERROR("Failed open GPIO chip(\"%s\") : %s \n",path.c_str(),strerror(errno));
		error = errno;
		goto cleanup;
	}
 
	// get refs to the lines
	error = gpiod_chip_get_lines(_chip, offsets, (unsigned int) pins.size() , &_lines);
	if(error) {
		LOGT_ERROR("Failed get GPIO lines: %s \n",strerror(errno));
		goto cleanup;
	}
	
	// setup the lines
	memset(&config, 0, sizeof(config));
	config.consumer = "test3";
	config.request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT;
	config.flags = 0;
	 
		// get the bulk lines setting default value to 0
	error = gpiod_line_request_bulk(&_lines, &config, initial_values);
	if(error) {
		LOGT_ERROR("Failed reequest GPIO lines: %s \n",strerror(errno));
	goto cleanup;
	}
 
	
	_isSetup = true;
	return _isSetup;
	
cleanup:
	 gpiod_line_release_bulk(&_lines);
	 gpiod_chip_close(_chip);
	_isSetup = false;

	return false;
}


void GPIO::stop(){

	if(_isSetup){
		gpiod_line_release_bulk(&_lines);
		gpiod_chip_close(_chip);
		_chip = NULL;
	}
	
	_isSetup = false;
}


bool GPIO::isAvailable(){
 
	return _isSetup;
}


bool GPIO::setRelays(gpioStates_t states){
	
	if(!_isSetup)
		return false;
	
	for(const auto& [relay, state] : states) {
		printf("Set GPIO(%2d), %s\n", relay, state?"ON":"OFF");
		
	}

	return true;

}

