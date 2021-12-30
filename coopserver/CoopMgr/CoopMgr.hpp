//
//  CoopMgr.hpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/20/21.
//

#ifndef CoopMgr_hpp
#define CoopMgr_hpp

#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <thread>			//Needed for std::thread
#include <functional>
#include <cstdlib>
#include <signal.h>

#include "CoopMgrCommon.h"
#include "CoopMgrDevice.hpp"
#include "CoopMgrDB.hpp"
#include "DoorMgr.hpp"

#include "CPUInfo.hpp"
#include "TempSensor.hpp"
#include "ScheduleMgr.hpp"
#include "CoopDevices.hpp"


using namespace std;
 
class CoopMgr {

public:
	static const char* 	CoopMgr_Version;

	static CoopMgr *shared() {
		if(!sharedInstance){
			sharedInstance = new CoopMgr;
		}
		return sharedInstance;
	}
 
	CoopMgr();
	~CoopMgr();
 
	void start();
	void stop();
	
	void startTempSensor( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);
	void stopTempSensor();
	CoopMgrDevice::device_state_t tempSensor1State();

	void startCoopDevices( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);
	void stopCoopDevices();
	CoopMgrDevice::device_state_t CoopDevicesState();
	
	string deviceStateString(CoopMgrDevice::device_state_t st);
 
	bool initDataBase(string schemaFilePath = "", string logDBFilePath = "");

	long upTime();  // how long since we started
	bool getSolarEvents(solarTimes_t &solar);

	CoopMgrDevice::device_state_t coopState() {return _state;};
	CoopMgrDB*		getDB() {return &_db; };
	
	void setActiveConnections(bool isActive);
	
	bool runAction(Action action,
						std::function<void(bool didSucceed)> callback = NULL);
	// events
	bool executeEvent(eventID_t eventID,
						 std::function<void(bool didSucceed)> callback = NULL);

	// coop door
	bool setDoor(bool isOpen, boolCallback_t callback = NULL);
	bool stopDoor(boolCallback_t callback = NULL);
	bool getDoor(std::function<void(bool didSucceed, DoorMgr::state_t state)> callback = NULL);
 
	// light state
	bool setLight(bool isOn, boolCallback_t callback = NULL);
	bool getLight(std::function<void(bool didSucceed, bool isOn)> callback = NULL);
	
 private:
	
	static CoopMgr *sharedInstance;
 
	CoopMgrDevice::device_state_t	_state;

	 bool 					_running;				//Flag for starting and terminating the main loop
	 std::thread 			_thread;				//Internal thread
	bool						_hasActiveConnections;
	
	 void run();

	CPUInfo				_cpuInfo;
	TempSensor			_tempSensor1;
	CoopDevices			_coopHW;
	CoopMgrDB			_db;
	

};
#endif /* CoopMgr_hpp */
