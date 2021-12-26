//
//  CoopMgr.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/20/21.
//

#include "CoopMgr.hpp"
#include "LogMgr.hpp"
#include "Utils.hpp"
 
 const char* 	CoopMgr::CoopMgr_Version = "1.0.0 dev 1";

CoopMgr *CoopMgr::sharedInstance = NULL;

static void sigHandler (int signum) {
	
	auto coopMgr = CoopMgr::shared();
	coopMgr->stop();
}

CoopMgr::CoopMgr(){
	
	signal(SIGKILL, sigHandler);
	signal(SIGHUP, sigHandler);
	signal(SIGQUIT, sigHandler);
	signal(SIGTERM, sigHandler);
//	signal(SIGINT, sigHandler);
 
	ScheduleMgr::shared();   // initialize the schedule manager - for uptime

	// start the thread running
	_running = true;
	_state = CoopMgrDevice::DEVICE_STATE_UNKNOWN;
	
	_thread = std::thread(&CoopMgr::run, this);
	_cpuInfo.begin();
}

CoopMgr::~CoopMgr(){
	_running = false;
	_cpuInfo.stop();
	
	if (_thread.joinable())
		_thread.join();
}


bool CoopMgr::initDataBase(string schemaFilePath,
										  string logDBFilePath ){

	bool success = false;
	_db.clear();
 
	_db.restorePropertiesFromFile();

	// setup logfile path
	string str;
	_db.getProperty(string(CoopMgrDB::PROP_LOG_FILE), &str);
	LogMgr::shared()->setLogFilePath(str);

	_db.getProperty(string(CoopMgrDB::PROP_LOG_FLAGS), &str);
	if(!str.empty()){
		char* p;
		long val = strtol(str.c_str(), &p, 0);
		if(*p == 0){
			LogMgr::shared()->_logFlags = (uint8_t)val;
		}
	}
		
	success =  _db.initSchemaFromFile(schemaFilePath)
					&& _db.initLogDatabase(logDBFilePath);

	if(success) {
		_state = CoopMgrDevice::DEVICE_STATE_DISCONNECTED;
		_db.logEvent(CoopMgrDB::EV_START );
		
	}
 
	return success;
}

void CoopMgr::start(){
	initDataBase();

 	startTempSensor();
	startCoopDevices([=](bool didSucceed, string error_text){
		
		if(didSucceed){
			_state = CoopMgrDevice::DEVICE_STATE_CONNECTED;
		}
		else {
			_state = CoopMgrDevice::DEVICE_STATE_ERROR;
		}
	});
 }

void CoopMgr::stop(){
 	stopTempSensor();
	stopCoopDevices();
	_state = CoopMgrDevice::DEVICE_STATE_DISCONNECTED;
}


void CoopMgr::setActiveConnections(bool isActive){
	_hasActiveConnections = isActive;
}


void CoopMgr::run(){
	
	constexpr long TIMEOUT_SEC = 3; //Timeout parameter for select() - in seconds
 
	try{
		
		while(_running){
			
			if(_tempSensor1.isConnected()){
				// handle input
				_tempSensor1.rcvResponse([=]( map<string,string> results){
 					_db.insertValues(results);
				});
			}
	 
			if(_cpuInfo.isConnected()){
				// handle input
				_cpuInfo.rcvResponse([=]( map<string,string> results){
	 				_db.insertValues(results);
				});
			}
		 
			if(_coopHW.isConnected()){
				// handle input
				_coopHW.rcvResponse([=]( map<string,string> results){
					_db.insertValues(results);
				});
			}
 
			sleep(TIMEOUT_SEC);
	 
			_tempSensor1.idle();
			_coopHW.idle();
			_cpuInfo.idle();
		};
	}
	catch ( const CoopException& e)  {
		printf("\tError %d %s\n\n", e.getErrorNumber(), e.what());
		
		if(e.getErrorNumber()	 == ENXIO){
	
		}
	}
}

// MARK: -   utilities

long CoopMgr::upTime(){
	return ScheduleMgr::shared()->upTime();
}

bool CoopMgr::getSolarEvents(solarTimes_t &solar){
	return ScheduleMgr::shared()->getSolarEvents(solar);
}


string CoopMgr::deviceStateString(CoopMgrDevice::device_state_t st) {
	switch( st){
		case CoopMgrDevice::DEVICE_STATE_UNKNOWN:
			return "Unknown";
		case CoopMgrDevice::DEVICE_STATE_DISCONNECTED:
			return "Disconnected";
		case CoopMgrDevice::DEVICE_STATE_CONNECTED:
			return "Connected";
		case CoopMgrDevice::DEVICE_STATE_ERROR:
			return "Error";
		case CoopMgrDevice::DEVICE_STATE_TIMEOUT:
			return "Timeout";

	}
};

// MARK: -   I2C Temp Sensors


void CoopMgr::startTempSensor( std::function<void(bool didSucceed, std::string error_text)> cb){
	
	int  errnum = 0;
	bool didSucceed = false;
	
	constexpr string_view TEMPSENSOR_KEY = "TEMP_";
	
	uint8_t deviceAddress = 0x48;
	string resultKey =  string(TEMPSENSOR_KEY) + to_hex(deviceAddress,true);
 
	didSucceed =  _tempSensor1.begin(deviceAddress, resultKey, errnum);
	if(didSucceed){
		_db.addSchema(resultKey,
						  CoopMgrDB::DEGREES_C,
						  "Coop Temperature",
						  CoopMgrDB::TR_TRACK);
		
		LOGT_DEBUG("Start TempSensor 1 - OK");
	}
	else
		LOGT_ERROR("Start TempSensor 1  - FAIL %s", string(strerror(errnum)).c_str());
 
	
	
	if(cb)
		(cb)(didSucceed, didSucceed?"": string(strerror(errnum) ));

}

void CoopMgr::stopTempSensor(){
	_tempSensor1.stop();
}

CoopMgrDevice::device_state_t CoopMgr::tempSensor1State(){
	return _tempSensor1.getDeviceState();
}


// MARK: -   Coop devices


void CoopMgr::startCoopDevices( std::function<void(bool didSucceed, std::string error_text)> cb){
	
	int  errnum = 0;
	bool didSucceed = false;
	
	
	didSucceed =  _coopHW.begin(errnum);
	if(didSucceed){
		LOGT_DEBUG("Start CoopDevices - OK");
	}
	else
		LOGT_ERROR("Start CoopDevices  - FAIL %s", string(strerror(errnum)).c_str());
	
	if(cb)
		(cb)(didSucceed, didSucceed?"": string(strerror(errnum) ));
}
 
void CoopMgr::stopCoopDevices(){
	_coopHW.stop();
}

CoopMgrDevice::device_state_t CoopMgr::CoopDevicesState(){
	return _coopHW.getDeviceState();
}


// coop door
bool CoopMgr::setDoor(bool isOpen, boolCallback_t cb){
	return _coopHW.setDoor(isOpen, cb);
}

bool CoopMgr::getDoor(std::function<void(bool didSucceed, CoopDevices::door_state_t state)> cb){
	return _coopHW.getDoor(cb);
}

 
// light state
bool CoopMgr::setLight(bool isOpen, boolCallback_t cb){
	return _coopHW.setLight(isOpen, cb);
}

bool CoopMgr::getLight(std::function<void(bool didSucceed, bool isOn)>cb){
	return _coopHW.getLight(cb);

}
 
