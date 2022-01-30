//
//  CoopMgr.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/20/21.
//

#include "CoopMgr.hpp"
#include "LogMgr.hpp"
#include "Utils.hpp"
 
 const char* 	CoopMgr::CoopMgr_Version = "1.0.0 dev 2";

CoopMgr *CoopMgr::sharedInstance = NULL;

static void sigHandler (int signum) {
	
	auto coopMgr = CoopMgr::shared();
	coopMgr->stop();
}

CoopMgr::CoopMgr(){
//	
 	signal(SIGKILL, sigHandler);
//	signal(SIGHUP, sigHandler);
//	signal(SIGQUIT, sigHandler);
//	signal(SIGTERM, sigHandler);
//	signal(SIGINT, sigHandler);
 
	_shouldRunStartupEvents = false;
	_shouldReconcileEvents	= false;

	SolarTimeMgr::shared();   // initialize the schedule manager - for uptime

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
	_db.resetAllEventsLastRun();

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
		_db.logHistoricalEvent(CoopMgrDB::EV_START );
		
	}
 
	return success;
}

void CoopMgr::start(){
	
	LOGT_DEBUG("Start Coop");

	initDataBase();

// 	startWittyPi3();
	startPiJuice();
	startTempSensor();
		
	startCoopDevices([=](bool didSucceed, string error_text){
		
		if(didSucceed){
			_state = CoopMgrDevice::DEVICE_STATE_CONNECTED;
 
			_shouldRunStartupEvents = true;
		}
		else {
			_state = CoopMgrDevice::DEVICE_STATE_ERROR;
		}
	});
 }

void CoopMgr::stop(){
	
	LOGT_DEBUG("Stop Coop");
 
	runShutdownEvents([=](){
//		stopWittyPi3();
		stopPiJuice();
		stopTempSensor();
		stopCoopDevices();
		_state = CoopMgrDevice::DEVICE_STATE_DISCONNECTED;
		_shouldRunStartupEvents = false;
		_shouldReconcileEvents = false;
		
	});
	
}

void CoopMgr::runShutdownEvents(std::function<void()> cb){
	auto eventIDs =  _db.eventsMatchingAppEvent(EventTrigger::APP_EVENT_SHUTDOWN);
	
	if(eventIDs.size() ==  0){
		if(cb) cb();
	}
	else {
		size_t* taskCount  = (size_t*) malloc(sizeof(size_t));
		*taskCount = eventIDs.size();

		// run startup events.
			for (auto eventID : eventIDs) {
			string name = 	_db.eventGetName(eventID);
			LOGT_INFO("RUN SHUTDOWN EVENT %04x - \"%s\"", eventID, name.c_str());

			executeEvent(eventID, [=]( bool didSucceed) {
				
				if(--(*taskCount) == 0) {
					free(taskCount);
					
					if(cb) cb();
				}
			});
		}
	}
 }


void CoopMgr::setActiveConnections(bool isActive){
	_hasActiveConnections = isActive;
}


void CoopMgr::run(){
	
	constexpr long SLEEP_SEC = 1;  // idle sleep in seconds
 
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
 
#ifdef  PIJUICE
		if(_piJuice.isConnected()){
				// handle input
			_piJuice.rcvResponse([=]( map<string,string> results){
					_db.insertValues(results);
				});
			}
#endif

#ifdef  WITTYPI3
		if(_wittyPi3.isConnected()){
				// handle input
				_wittyPi3.rcvResponse([=]( map<string,string> results){
					_db.insertValues(results);
				});
			}
#endif
			sleep(SLEEP_SEC);
	 
			_tempSensor1.idle();

#ifdef  WITTYPI3
			_wittyPi3.idle();
#endif
			
#ifdef  PIJUICE
			_piJuice.idle();
#endif
			
			_coopHW.idle();
			_cpuInfo.idle();
			
			idleLoop();
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
	return SolarTimeMgr::shared()->upTime();
}

bool CoopMgr::getSolarEvents(solarTimes_t &solar){
	return SolarTimeMgr::shared()->getSolarEvents(solar);
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

// MARK: -   I2C PiJuice

 
void CoopMgr::startPiJuice( std::function<void(bool didSucceed, std::string error_text)> cb){
	
	int  errnum = 0;
	bool didSucceed = false;
 
#ifdef  PIJUICE

	didSucceed =  _piJuice.begin(errnum);
	if(didSucceed){

		LOGT_DEBUG("Start PiJuice - OK");
	}
	else
		LOGT_ERROR("Start PiJuice - FAIL %s", string(strerror(errnum)).c_str());
#endif
 
	if(cb)
		(cb)(didSucceed, didSucceed?"": string(strerror(errnum) ));
}

void CoopMgr::stopPiJuice(){
#ifdef  PIJUICE
	_piJuice.stop();
#else
#endif
}

CoopMgrDevice::device_state_t CoopMgr::PiJuiceState(){
#ifdef  PIJUICE
	return _piJuice.getDeviceState();
#else
	return CoopMgrDevice::DEVICE_STATE_DISCONNECTED;
#endif

}


// MARK: -   I2C WittyPi3

void CoopMgr::startWittyPi3( std::function<void(bool didSucceed, std::string error_text)> cb){
	
	int  errnum = 0;
	bool didSucceed = false; 
 
#ifdef  WITTYPI3

	didSucceed =  _wittyPi3.begin(errnum);
	if(didSucceed){
//		_db.addSchema(resultKey,
//						  CoopMgrDB::DEGREES_C,
//						  "Coop Temperature",
//						  CoopMgrDB::TR_TRACK);
//
		LOGT_DEBUG("Start WittyPi3 - OK");
	}
	else
		LOGT_ERROR("Start WittyPi3 - FAIL %s", string(strerror(errnum)).c_str());
#endif
 
	if(cb)
		(cb)(didSucceed, didSucceed?"": string(strerror(errnum) ));
}

void CoopMgr::stopWittyPi3(){
#ifdef  WITTYPI3
	_wittyPi3.stop();
#else
#endif
}

CoopMgrDevice::device_state_t CoopMgr::wittyPi3tate(){
#ifdef  WITTYPI3
	return _wittyPi3.getDeviceState();
#else
	return CoopMgrDevice::DEVICE_STATE_DISCONNECTED;
#endif

}

bool CoopMgr::getVoltageIn(double &val){
#ifdef  WITTYPI3
	return _wittyPi3.voltageIn(val);
#else
	return false;
#endif
 }

bool CoopMgr::getVoltageOut(double &val){
#ifdef  WITTYPI3
	return _wittyPi3.voltageOut(val);
#else
	return false;
#endif
 }

bool CoopMgr::getCurrentOut(double &val) {
#ifdef  WITTYPI3
	return _wittyPi3.currentOut(val);
#else
	return false;
#endif
}

bool CoopMgr::getPowerMode(bool &val){
#ifdef  WITTYPI3
	return _wittyPi3.powerMode(val);
#else
	return false;
#endif
}

bool CoopMgr::getPowerTemp(double &val){
#ifdef  WITTYPI3
	return _wittyPi3.tempC(val);
#else
	return false;
#endif
}


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
						  CoopMgrDB::DEGREES_C,2,
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
	if(!didSucceed) 
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

// MARK: Coop door
 
bool CoopMgr::setDoor(bool isOpen, boolCallback_t cb){
	
	LOGT_INFO("DOOR %s", isOpen?"OPEN":"CLOSE");

	if(isOpen) {
		return _coopHW.doorOpen(cb);
	}
	else {
		return _coopHW.doorClose(cb);
	}
}

bool CoopMgr::stopDoor(boolCallback_t cb){
	return _coopHW.doorStop(cb);
}

bool CoopMgr::getDoor(std::function<void(bool didSucceed, DoorMgr::state_t state)> cb){
	return _coopHW.getDoor(cb);
}
 
// MARK: Coop light

bool CoopMgr::setLight(bool isOpen, boolCallback_t cb){
	
	LOGT_INFO("LIGHT %s", isOpen?"ON":"OFF");
	return _coopHW.setLight(isOpen, cb);
}

bool CoopMgr::getLight(std::function<void(bool didSucceed, bool isOn)>cb){
	return _coopHW.getLight(cb);
}

// MARK: Aux Relay

bool CoopMgr::setAux(bool isOpen, boolCallback_t cb){
	
	LOGT_INFO("AUX RELAY %s", isOpen?"ON":"OFF");
	return _coopHW.setAux(isOpen, cb);
}

bool CoopMgr::getAux(std::function<void(bool didSucceed, bool isOn)>cb){
	return _coopHW.getAux(cb);
}
 

// MARK: Combined coop state

bool CoopMgr::getCoopState(std::function<void(bool didSucceed, CoopMgr::coopState_t coopState)> cb){
	
	CoopMgr::coopState_t coopState =
	{.doorstate = DoorMgr::STATE_UNKNOWN,
		.lightState	= false,
		.auxState = false,
		.coopTempC = 0.0
#ifdef  PIJUICE
		,
		.pjStatus.byteWrapped = 0,
		.pjFault.byteWrapped = 0,
		.battery_soc = 0
#endif
	} ;
	
	return _coopHW.getDoor([&coopState, cb, this] (bool didSucceed, DoorMgr::state_t doorState ){
		
		if(didSucceed) {
			coopState.doorstate = doorState;
			
			_coopHW.getLight([&coopState, cb, this] (bool didSucceed,bool isOn ){
				if(didSucceed) {
					coopState.lightState = isOn;
					
					_coopHW.getAux([&coopState, cb, this] (bool didSucceed,bool isOn ){
						if(didSucceed) {
							coopState.auxState = isOn;
							
							bool status =  _tempSensor1.tempC(coopState.coopTempC);
#ifdef  PIJUICE
							status &= _piJuice.SOC(coopState.battery_soc);
							status &= _piJuice.status(coopState.pjStatus, coopState.pjFault);
#endif
							if(cb) (cb)( status, coopState);
						}
						else {
							if(cb) (cb)( false,coopState);
						}
					});
				}
				else {
					if(cb) (cb)( false,coopState);
				}
				
			});
		}
		else {
			if(cb) (cb)( false,coopState);
			
		}
	});
}



//MARK: -  events / actions

bool CoopMgr::executeEvent(eventID_t eventID,
										std::function<void(bool didSucceed)> cb){
 	if(!_db.eventsIsValid(eventID))
		return false;
	
	auto ref = _db.eventsGetEvent(eventID);
	if(!ref)
		return false;
	

	Event event = ref->get();
	Action action = event.getAction();
	string name = event.getName();
	
	LOGT_INFO("RUN EVENT %04x - \"%s\"", eventID, name.c_str());

	bool handled = runAction(action, [=](bool didSucceed){
		if(cb) (cb)( didSucceed);
	});


	return handled;
}

bool CoopMgr::runAction(Action action,
									std::function<void(bool didSucceed)> cb){
	
	bool handled = false;
	bool relayState = false;
 
 	if(action.deviceID() == JSON_DEVICE_DOOR){
		
		if( CoopDevices::stringToRelayState(action.cmd(), &relayState)){
			handled = setDoor(relayState, cb);
		}
	}
	else if(action.deviceID() == JSON_DEVICE_LIGHT){
		
		if( CoopDevices::stringToRelayState(action.cmd(), &relayState)){
			handled = setLight(relayState , cb);
 		}
 	}
	else if(action.deviceID() == JSON_DEVICE_AUX){
		if( CoopDevices::stringToRelayState(action.cmd(), &relayState)){
			handled = setAux(relayState , cb);
		}
	}
	
	return handled;
}
//MARK: -  idle loop

void CoopMgr::idleLoop() {

	// limit this twice a minute
		
	static time_t lastRun = 0;

	time_t now = time(NULL);
	struct tm* tm = localtime(&now);
	time_t localNow  = (now + tm->tm_gmtoff);

	if(localNow > (lastRun + 30 ))  // 30 seconds delay
	{
		lastRun = localNow;
		
 		if(_state == CoopMgrDevice::DEVICE_STATE_CONNECTED) {
		
		// process startup events first
			if(_shouldRunStartupEvents) {
				
				auto eventIDs =  _db.eventsMatchingAppEvent(EventTrigger::APP_EVENT_STARTUP);
				
				if(eventIDs.size() > 0){
					size_t* taskCount  = (size_t*) malloc(sizeof(size_t));
					*taskCount = eventIDs.size();
		 
					// run startup events.
			 			for (auto eventID : eventIDs) {
						string name = 	_db.eventGetName(eventID);
						LOGT_INFO("RUN STARTUP EVENT %04x - \"%s\"", eventID, name.c_str());

						executeEvent(eventID, [=]( bool didSucceed) {
							
							if(--(*taskCount) == 0) {
								free(taskCount);
								
								_shouldReconcileEvents = true;
							}
						});
					}
				}
				else {
					_shouldReconcileEvents = true;
				}
				
				_shouldRunStartupEvents = false;
			}
	
			// good place to check for events.
			solarTimes_t solar;
			if(SolarTimeMgr::shared()->getSolarEvents(solar)){
				
				// combine any unrun events.
				if(_shouldReconcileEvents) {
					_db.reconcileEventGroups(solar, localNow);
					_shouldReconcileEvents = false;
				}
				
				auto eventIDs = _db.eventsThatNeedToRun(solar, localNow);
				
				for (auto eventID : eventIDs) {
					string name = 	_db.eventGetName(eventID);
					LOGT_INFO("RUN EVENT %04x - \"%s\"", eventID, name.c_str());

					executeEvent(eventID, [=]( bool didSucceed) {
						_db.eventSetLastRunTime(eventID, localNow);
					});
				}
			}
 		}
	}
 };

extern "C" {


void dumpHex(uint8_t* buffer, int length, int offset)
{
	char hexDigit[] = "0123456789ABCDEF";
	int			i;
	int						lineStart;
	int						lineLength;
	short					c;
	const unsigned char	  *bufferPtr = buffer;
	
	char                    lineBuf[1024];
	char                    *p;
	
#define kLineSize	8
	for (lineStart = 0, p = lineBuf; lineStart < length; lineStart += lineLength,  p = lineBuf )
	{
		lineLength = kLineSize;
		if (lineStart + lineLength > length)
			lineLength = length - lineStart;
		
		p += sprintf(p, "%6d: ", lineStart+offset);
		for (i = 0; i < lineLength; i++){
			*p++ = hexDigit[ bufferPtr[lineStart+i] >>4];
			*p++ = hexDigit[ bufferPtr[lineStart+i] &0xF];
			if((lineStart+i) &0x01)  *p++ = ' ';  ;
		}
		for (; i < kLineSize; i++)
			p += sprintf(p, "   ");
		
		p += sprintf(p,"  ");
		for (i = 0; i < lineLength; i++) {
			c = bufferPtr[lineStart + i] & 0xFF;
			if (c > ' ' && c < '~')
				*p++ = c ;
			else {
				*p++ = '.';
			}
		}
		*p++ = 0;
		
		
		printf("%s\n",lineBuf);
	}
#undef kLineSize
}
}
