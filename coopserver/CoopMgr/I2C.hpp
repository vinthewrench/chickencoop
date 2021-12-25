//
//  I2C.hpp
//  coopMgr
//
//  Created by Vincent Moscaritolo on 9/10/21.
//

#ifndef I2C_hpp
#define I2C_hpp

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

class I2C  {
	
	
public:
	I2C();
	~I2C();
	
	bool begin(uint8_t	devAddr);
 	bool begin(uint8_t	devAddr,  int &error);

	void stop();

	bool isAvailable();
	
	ssize_t readBytes(uint8_t regAddr, void *buf, size_t nbyte);
	ssize_t readByte(uint8_t regAddr,  void *buf);
	ssize_t readBytes(void *buf, size_t nbyte);
	ssize_t readByte(void *buf);

	ssize_t writeBytes(const uint8_t* buf, size_t nbyte);

	ssize_t writeBytes(uint8_t regAddr, const uint8_t* buf, size_t nbyte);
	ssize_t writeByte(uint8_t regAddr, const uint8_t data);
	ssize_t writeByte(const uint8_t data);
	
	uint8_t	getDevAddr() {return _devAddr;};
	
private:
	int 			_fd;
	int 			_devAddr;
	bool 			_isSetup;
 
};

 
#endif /* I2C_hpp */
