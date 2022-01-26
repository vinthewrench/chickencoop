//
//  RelayHat.hpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/23/21.
//

#ifndef RelayHat_hpp
#define RelayHat_hpp
 
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
	Driver for keyestudio RPI 4-channel Relay Shield
 
 https://wiki.keyestudio.com/KS0212_keyestudio_RPI_4-channel_Relay_Shield

 */
 
using namespace std;


class RelayHat  {
	
public:
	
	typedef enum {
		CH1	= 26,
		CH2	= 6,
		CH3 	= 22,
		CH4	= 4,
		}relaysID_t;

	typedef vector<pair<RelayHat::relaysID_t, bool>> relayStates_t;

	RelayHat();
	~RelayHat();

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

#endif /* RelayHat_hpp */
