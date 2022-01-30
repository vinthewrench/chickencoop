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
   
	typedef union {
		 struct
		 {
			  bool isFault : 1;
			 //  True if there are faults or fault events waiting to be read or False if there are no faults and no fault events.

			  bool isButton : 1;
			 //True if there are button events, False if not.
			 
			 unsigned int battery_status: 2;
			//  current battery status, one of four: 'NORMAL', 'CHARGING_FROM_IN', 'CHARGING_FROM_5V_IO', 'NOT_PRESENT'.
		
			 unsigned int power_input_status: 2;
			//  current status of USB Micro power input, one of four: 'NOT_PRESENT', 'BAD', 'WEAK', 'PRESENT'.
			 
			 unsigned int FiveV_power_input_status: 2;
			 //  current status of USB Micro power input, one of four: 'NOT_PRESENT', 'BAD', 'WEAK', 'PRESENT'.
		 };
		 uint8_t byteWrapped;
	} piStatus_t;

	typedef union {
		 struct
		{
			 bool button_power_off : 1;
			 bool forced_power_off : 1;
			 bool forced_sys_power_off : 1;
			 bool watchdog_reset : 1;
			 
			 bool unused : 1;
			 bool battery_profile_invalid : 1;
			 unsigned int charging_temperature_fault: 2;
			//  'NORMAL', 'SUSPEND', 'COOL', 'WARM'
		 };
		 uint8_t byteWrapped;
	} piFault_t;
	
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
	bool status(piStatus_t &status,  piFault_t &fault);

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
