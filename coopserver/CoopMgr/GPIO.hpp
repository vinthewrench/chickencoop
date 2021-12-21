//
//  GPIO.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 12/21/21.
//

#ifndef GPIO_hpp
#define GPIO_hpp
 
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <time.h>
#include <termios.h>
#include <stdexcept>
#include <string>

using namespace std;

class GPIO  {
	
public:
	GPIO();
	~GPIO();

	bool begin(string	path);
	bool begin(string	path,  int *error = NULL);

	void stop();

	bool isAvailable();

private:
 
	struct gpiod_chip* _chip;
	bool 					_isSetup;

};

#endif /* GPIO_hpp */
