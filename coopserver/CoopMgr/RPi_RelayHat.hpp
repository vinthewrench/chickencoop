//
//  RPi_RelayHat.hpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/23/21.
//

#ifndef RPi_RelayHat_hpp
#define RPi_RelayHat_hpp
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <time.h>
#include <termios.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <tuple>

#include "GPIO.hpp"

/*
 	Driver for waveshare RPi_Relay_Board
 
	https://www.waveshare.com/wiki/RPi_Relay_Board

 */
 
using namespace std;


class RPi_RelayHat  {
	
public:
	
	typedef enum {
		CH1	= 26,
		CH2	= 22,
		CH3 	= 6,
		CH4	= 4,
		}relaysID_t;

	typedef vector<pair<RPi_RelayHat::relaysID_t, bool>> relayStates_t;

	RPi_RelayHat();
	~RPi_RelayHat();

	bool begin(string	path);
 	bool begin(string	path, int &error);

	void stop();

	bool isAvailable();
	
	bool setRelays(relayStates_t states);

	bool getRelays(relayStates_t &states);

private:
 
  	bool 				_isSetup;

	GPIO				_gpio;
};

#endif /* RPi_RelayHat_hpp */
