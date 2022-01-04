//
//  GPIO.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 12/21/21.
//

#include "GPIO.hpp"
#include <algorithm>


#include "LogMgr.hpp"

GPIO::GPIO(){
	_isSetup = false;
 }


GPIO::~GPIO(){
	stop();
	
}
 
bool GPIO::begin(string	path, vector<uint8_t> lines, int request_type){
	int error = 0;
	
	int flags = 0;

	return begin(path, lines, request_type, flags, {}, error);
}


bool GPIO::begin(string	path, vector<uint8_t> pins, int request_type,  int flags,  vector<bool> initialValue,  int &error){
 
	struct gpiod_line_request_config config;

	// Create an array of size equivalent to vector
	 unsigned int offsets[ pins.size()];
	  // Copy all elements of vector to array
	  std::transform( pins.begin(),  pins.end(),  offsets, [](const auto & elem){  return elem; });
  
	_pins = pins;		// remember the order of the pins for later

	int initial_values[ initialValue.size()];
	 // Copy all elements of vector to array
	 std::transform( initialValue.begin(),  initialValue.end(),  initial_values, [](const auto & elem){  return elem; });
	
// debug
//	for(auto x: offsets) { 	printf("Setup GPIO(%2d)\n", x); }
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
	config.consumer = "GPIO";
	config.request_type = request_type;
	config.flags = flags;
	 
		// get the bulk lines setting default value to 0
	error = gpiod_line_request_bulk(&_lines, &config, initial_values);
	if(error) {
		LOGT_ERROR("Failed request GPIO lines: %s \n",strerror(errno));
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
	
	int values[_pins.size()];
	
	if(!_isSetup)
		return false;
	
	int  error = 0;
	
	// get the current state of lines
	error = gpiod_line_get_value_bulk(&_lines, values);
	if(error) {
		LOGT_ERROR("Failed get line values GPIO lines: %s \n",strerror(errno));
		return false;
	}
	
	// update the values to match what we are asking for.

	for(const auto& [relay, state] : states) {
		std::vector<uint8_t>::iterator it = std::find(_pins.begin(), _pins.end(), relay);
		if(it != _pins.end()){
			int index = int(std::distance(_pins.begin(), it));
			values[index] = state?1:0;
		}
	}
	
	// set the lines to match values
	error = gpiod_line_set_value_bulk(&_lines, values);
	if(error) {
		LOGT_ERROR("Failed get set values GPIO lines: %s \n",strerror(errno));
		return false;
	}

	return true;
	
}


bool GPIO::getRelays(gpioStates_t &states){

	int values[_pins.size()];
	int  error = 0;

	if(!_isSetup)
		return false;
	
	states.clear();
	// get the current state of lines
	error = gpiod_line_get_value_bulk(&_lines, values);
	if(error) {
		LOGT_ERROR("Failed get line values GPIO lines: %s \n",strerror(errno));
		return false;
	}
	
	for(int i = 0; i < _pins.size(); i++){
		states.push_back(make_pair(_pins[i],values[i] == 0?false:true));
	}
	return true;
 }
