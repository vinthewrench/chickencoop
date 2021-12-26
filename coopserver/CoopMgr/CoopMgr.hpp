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

#include "CPUInfo.hpp"
#include "TempSensor.hpp"
#include "ScheduleMgr.hpp"
#include "CoopDevices.hpp"
#include "QwiicButton.hpp"


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
	
	
	// coop door
	bool setDoor(bool isOpen, boolCallback_t callback = NULL);
	CoopDevices::door_state_t getDoorState();
	
	// light state
	bool setLight(bool isOn, boolCallback_t callback = NULL);
	bool getLight();

	
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
	
	QwiicButton			_greenButton;
	QwiicButton			_redButton;

};
#endif /* CoopMgr_hpp */