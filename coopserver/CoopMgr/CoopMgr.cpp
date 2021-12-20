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



CoopMgr::CoopMgr()
//:
//								_tankSensor(&_db),
//								_inverter(&_db),
//								_pumpSensor(&_db)
{
//	signal(SIGINT, signal_callback_handler);
//	signal(SIGHUP, signal_callback_handler);
//	signal(SIGQUIT, signal_callback_handler);
//	signal(SIGTERM, signal_callback_handler);
//	signal(SIGKILL, signal_callback_handler);
//
	
	// start the thread running
	_running = true;
	_state = CoopMgrDevice::DEVICE_STATE_UNKNOWN;
	_startTime = time(NULL);
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
 }

void CoopMgr::stop(){
 	stopTempSensor();
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

		
	
			sleep(TIMEOUT_SEC);
	 
			_tempSensor1.idle();
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
	time_t now = time(NULL);

	return now - _startTime;
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
 
	didSucceed =  _tempSensor1.begin(deviceAddress,resultKey, &errnum);
	if(didSucceed){
//		_db.addSchema(resultKey,
//						  PumpHouseDB::DEGREES_C,
//						  "Inverter Temperature",
//						  PumpHouseDB::TR_TRACK);
		
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
 
