//
//  WittyPi3.hpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 1/4/22.
//

#ifndef WittyPi3_hpp
#define WittyPi3_hpp
 
#include <stdio.h>
#include <functional>
#include <string>
#include <sys/time.h>
#include "I2C.hpp"

#include <map>
#include "CoopMgrDevice.hpp"
 
using namespace std;

class WittyPi3 : public CoopMgrDevice{
 
public:
	
	struct registermap {
		uint8_t id;							//firmware id
		uint8_t voltage_in_i;			//integer part for input voltage
		uint8_t voltage_in_d;			//decimal part (x100) for input voltage
		uint8_t voltage_out_i;			//integer part for output voltage
		uint8_t voltage_out_d;			//decimal part (x100) for output voltage
		uint8_t current_out_i;			//integer part for output current
		uint8_t current_out_d;			//decimal part (x100) for output current
		uint8_t power_mode;				//1 if Witty Pi is powered via the LDO, 0 if direclty use 5V input
		uint8_t lv_shutdown;				// 1 if system was shutdown by low voltage, otherwise 0
		uint8_t conf_address;			//I2C slave address: defaul=0x69
		uint8_t conf_default_on;		//turn on RPi when power is connected: 1=yes, 0=no
		uint8_t conf_pulse_interval;	//pulse interval (for LED and dummy load): 9=8s,8=4s,7=2s,6=1s
		uint8_t conf_low_voltage;		//low voltage threshold (x10), 255=disabled
		uint8_t conf_blink_led;			//0 if white LED should not blink. The bigger value, the longer time to light up LED
		uint8_t conf_power_cut_delay;	//the delay (x10) before power cut: default=50 (5 sec)
		uint8_t conf_recovery_voltage;//voltage (x10) that triggers recovery, 255=disabled
		uint8_t conf_dummy_load;		//0 if dummy load is off. The bigger value, the longer time to draw current
		uint8_t conf_adj_vin;			//adjustment for measured Vin (x100), range from -127 to 127
		uint8_t conf_adj_vout;			//adjustment for measured Vout (x100), range from -127 to 127
		uint8_t conf_adj_iout;			//adjustment for measured Iout (x100), range from -127 to 127
	};

	WittyPi3();
	~WittyPi3();

	bool begin();
	bool begin(int &error);
	void stop();

	bool isConnected();
 
	response_result_t rcvResponse(std::function<void(map<string,string>)> callback = NULL);
	
	void idle(); 	// called from loop
	void reset(); 	// reset from timeout

 	device_state_t getDeviceState();
	
	bool getRegisters(registermap &regs);
	bool voltageIn(double &val);
	bool voltageOut(double &val);
	bool currentOut(double &val);
	bool powerMode(bool &val);
	
	bool  tempC(double &val);
	bool  tempF(double &val);
	

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
};
 
#endif /* WittyPi3_hpp */
