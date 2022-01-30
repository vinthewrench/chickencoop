//
//  PiJuice.hpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 1/29/22.
//

#ifndef PiJuice_hpp
#define PiJuice_hpp


#include <stdio.h>
#include <functional>
#include <string>
#include <sys/time.h>
#include "I2C.hpp"

#include <map>
#include "CoopMgrDevice.hpp"

using namespace std;

class PiJuice : public CoopMgrDevice{

public:
   
	PiJuice();
  ~PiJuice();

  bool begin();
  bool begin(int &error);
  void stop();

  bool isConnected();

  response_result_t rcvResponse(std::function<void(map<string,string>)> callback = NULL);
  
  void idle(); 	// called from loop
  void reset(); 	// reset from timeout

  device_state_t getDeviceState();

	bool batteryVoltage(double &val);
	bool currentOut(double &val);

	bool SOC(double &val);	// state of charge
	bool status();

	bool  tempC(double &val);

private:

  typedef enum  {
	  INS_UNKNOWN = 0,
	  INS_IDLE ,
	  INS_INVALID,
	  INS_RESPONSE,
  }in_state_t;

  
  in_state_t 		_state;
  map<string,string> _resultMap;

  timeval			_lastQueryTime;
  uint64_t     	_queryDelay;			// how long to wait before next query

  I2C 		_i2cPJ;
  
  bool		_isSetup;

};

#endif /* PiJuice_hpp */
