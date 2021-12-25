//
//  CoopMgrCommon.h
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/20/21.
//

#ifndef coopMgrCommon_h
#define coopMgrCommon_h

#include <stdexcept>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <functional>


typedef std::function<void(bool didSucceed)> boolCallback_t;

extern "C" {
void dumpHex(uint8_t* buffer, int length, int offset = 0);
}

class CoopException: virtual public std::runtime_error {
	 
protected:

	 int error_number;               ///< Error Number
	 
public:

	 /** Constructor (C++ STL string, int, int).
	  *  @param msg The error message
	  *  @param err_num Error number
	  */
	 explicit
	CoopException(const std::string& msg, int err_num = 0):
		  std::runtime_error(msg)
		  {
				error_number = err_num;
		  }

	
	 /** Destructor.
	  *  Virtual to allow for subclassing.
	  */
	 virtual ~CoopException() throw () {}
	 
	 /** Returns error number.
	  *  @return #error_number
	  */
	 virtual int getErrorNumber() const throw() {
		  return error_number;
	 }
};
 

#endif /* coopMgrCommon_h */
