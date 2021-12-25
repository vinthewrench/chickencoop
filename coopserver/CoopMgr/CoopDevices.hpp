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
#include "json.hpp"

#include "CoopMgrDevice.hpp"
#include "RPi_RelayHat.hpp"

using namespace std;
 

class CoopDevices : public CoopMgrDevice{
	
public:
	
	typedef enum  {
		STATE_UNKNOWN = 0,
		STATE_OPEN,
		STATE_OPENING,
		STATE_CLOSED,
		STATE_CLOSING,
	}door_state_t;
	
	CoopDevices();
	~CoopDevices();
	
	bool begin( int &error);
	void stop();
	
	bool isConnected();
	
	void idle(); 	// called from loop
	void reset(); 	// reset from timeout
	
	device_state_t getDeviceState();
	
	// door state
	bool setDoor(bool isOpen, boolCallback_t callback = NULL);
	door_state_t getDoorState();
	
	// light state
	bool setLight(bool isOn, boolCallback_t callback = NULL);
	bool getLight();
		
	static bool stringToRelayState(const std::string str, bool* stateOut = NULL);
	static bool jsonToRelayState( nlohmann::json j, bool* stateOut = NULL);
 
	
private:
	
	typedef enum  {
		INS_UNKNOWN = 0,
		INS_IDLE ,
		INS_INVALID,
		INS_RESPONSE,
		
	}in_state_t;
	
	in_state_t 		_state;
	RPi_RelayHat	_relay;
};
#endif /* CoopDevices_hpp */
