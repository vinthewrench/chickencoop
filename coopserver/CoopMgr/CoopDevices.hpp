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
#include "DoorMgr.hpp"
#include "QwiicButton.hpp"

using namespace std;
 

constexpr string_view COOP_DEVICE_LIGHT_STATE	= "LIGHT_STATE";
constexpr string_view COOP_DEVICE_DOOR_STATE		= "DOOR_STATE";

class CoopDevices : public CoopMgrDevice{
	friend DoorMgr;
	
public:
	
 
	CoopDevices();
	~CoopDevices();
	
	bool begin( int &error);
	void stop();
	
	bool isConnected();

	response_result_t rcvResponse(std::function<void(map<string,string>)> callback = NULL);

	void idle(); 	// called from loop
	void reset(); 	// reset from timeout
	
	device_state_t getDeviceState();
	

	// initiate change in door state
	bool doorOpen(boolCallback_t callback = NULL);
	bool doorClose(boolCallback_t callback = NULL);
	bool doorStop(boolCallback_t callback = NULL);

	// get the actual bits
	bool getDoor(std::function<void(bool didSucceed, DoorMgr::state_t state)> callback = NULL);
 
	// light state
	bool setLight(bool isOn, boolCallback_t callback = NULL);
	bool getLight(std::function<void(bool didSucceed, bool isOn)> callback = NULL);
 
	static bool stringToRelayState(const std::string str, bool* stateOut = NULL);
	static bool jsonToRelayState( nlohmann::json j, bool* stateOut = NULL);
 
	
private:
	
	// call back from DoorMgr
	bool setDoor(bool isOpen, boolCallback_t callback = NULL);
	bool stopDoor(boolCallback_t callback = NULL);

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

	RPi_RelayHat	_relay;
	DoorMgr 			_doorMgr;
	QwiicButton			_greenButton;
	QwiicButton			_redButton;

};
#endif /* CoopDevices_hpp */
