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
	typedef uint8_t  i2c_block_t [32];
	
	I2C();
	~I2C();
	
	bool begin(uint8_t	devAddr);
 	bool begin(uint8_t	devAddr,  int &error);

	void stop();

	bool 		isAvailable();
	uint8_t	getDevAddr() {return _devAddr;};
	
	bool writeByte(uint8_t regAddr, uint8_t byte);
	bool writeWord(uint8_t regAddr, uint16_t word);

	bool readByte(uint8_t regAddr,  uint8_t& byte);
	bool readWord(uint8_t regAddr,  uint16_t& word);

	bool readBlock(uint8_t regAddr, uint8_t size, i2c_block_t & block );
		
private:

	int 			_fd;
	int 			_devAddr;
	bool 			_isSetup;
 
};

 
#endif /* I2C_hpp */
