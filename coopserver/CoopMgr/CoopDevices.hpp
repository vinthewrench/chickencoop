//
//  CoopDevices.hpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/21/21.
//

#ifndef CoopDevices_hpp
#define CoopDevices_hpp
 
#include <stdio.h>
#include <functional>
#include <string>
#include <sys/time.h>

#include <map>
#include "CoopMgrDevice.hpp"
#include "GPIO.hpp"

using namespace std;

class CoopDevices : public CoopMgrDevice{

public:
	CoopDevices();
  ~CoopDevices();

  bool begin( int *error = NULL);
  void stop();

  bool isConnected();
  
  void idle(); 	// called from loop
  void reset(); 	// reset from timeout

  device_state_t getDeviceState();
  
private:

  typedef enum  {
	  INS_UNKNOWN = 0,
	  INS_IDLE ,
	  INS_INVALID,
	  INS_RESPONSE,
	
  }in_state_t;
  
  in_state_t 		_state;
	GPIO				_gpio;
};
#endif /* CoopDevices_hpp */
