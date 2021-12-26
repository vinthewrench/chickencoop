//
//  GPIO.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 12/21/21.
//

#ifndef GPIO_hpp
#define GPIO_hpp
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <time.h>
#include <termios.h>
#include <stdexcept>
#include <string>
#include <unistd.h>

#include <vector>
#include <tuple>

#if defined(__APPLE__)
// used for cross compile on osx
#include "macos_gpiod.h"
#else
#include <gpiod.h>
#endif

using namespace std;

class GPIO  {
	
public:
	
	typedef vector<pair<uint8_t, bool>> gpioStates_t;

	GPIO();
	~GPIO();

	bool begin(string	path, vector<uint8_t> pins, int request_type);
	bool begin(string	path, vector<uint8_t> pins, int request_type, vector<bool> initialValue, int  &error);
 	void stop();

	bool isAvailable();

	bool setRelays(gpioStates_t states);

private:
 
	vector<uint8_t> 			_pins;
	struct gpiod_chip* 		_chip;
	struct gpiod_line_bulk  _lines;
	  
	bool 					_isSetup;

};

#endif /* GPIO_hpp */
