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


using namespace std;
 
class CoopMgr {

public:
	static const char* 	CoopMgr_Version;

	CoopMgr();
	~CoopMgr();
 
	void start();
	void stop();
	
	void startTempSensor( std::function<void(bool didSucceed, std::string error_text)> callback = NULL);
	void stopTempSensor();
	CoopMgrDevice::device_state_t tempSensor1State();


	
	string deviceStateString(CoopMgrDevice::device_state_t st);
 
	bool initDataBase(string schemaFilePath = "", string logDBFilePath = "");

	long upTime();  // how long since we started
	
	CoopMgrDevice::device_state_t coopState() {return _state;};
	CoopMgrDB*		getDB() {return &_db; };
	
	void setActiveConnections(bool isActive);
	
 private:
	 
	CoopMgrDevice::device_state_t	_state;

	 bool 					_running;				//Flag for starting and terminating the main loop
	 std::thread 			_thread;				//Internal thread
	bool						_hasActiveConnections;
	
	 void run();

	time_t 				_startTime;		// to calculate uptime
	CPUInfo				_cpuInfo;
	TempSensor			_tempSensor1;

	CoopMgrDB			_db;

};
#endif /* CoopMgr_hpp */
