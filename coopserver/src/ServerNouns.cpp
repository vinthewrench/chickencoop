//
//  ServerNouns.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/20/21.
//

#include <stdio.h>
#include <iostream>
#include <chrono>

#include "ServerCmdQueue.hpp"
#include "CmdLineHelp.hpp"
#include <regex>
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <unistd.h>			//Needed for access()
#include <string>
#include <string.h>
#include <map>
#include <sys/utsname.h>

#include "Utils.hpp"
#include "TimeStamp.hpp"

#include "TCPServer.hpp"
#include "REST/RESTServerConnection.hpp"
#include "ServerCmdQueue.hpp"

#include "ServerCommands.hpp"
#include "ServerCmdValidators.hpp"
#include "CommonIncludes.hpp"

#include "LogMgr.hpp"
#include "Utils.hpp"


static bool getCPUTemp(double &tempOut) {
	bool didSucceed = false;

//#if defined(__PIE__)
	// return the CPU temp
		{
			try{
				std::ifstream   ifs;
				ifs.open("/sys/class/thermal/thermal_zone0/temp", ios::in);
				if( ifs.is_open()){
					string val;
					ifs >> val;
					ifs.close();
					double temp = std::stod(val);
					temp = temp /1000.0;
					tempOut = temp;
					didSucceed = true;
				}
				
			}
			catch(std::ifstream::failure &err) {
			}
		}
//#else
//	tempOut = 38.459;
//	didSucceed = true;

//#endif
	
	return didSucceed;
}


// MARK:  DEVICES NOUN HANDLERS

static bool Devices_NounHandler_GET(ServerCmdQueue* cmdQueue,
												REST_URL url,
												TCPClientInfo cInfo,
												ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	using namespace timestamp;
	
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
 
 
	json reply;

	
	return false;

}



static bool Devices_NounHandler_PUT(ServerCmdQueue* cmdQueue,
												REST_URL url,
												TCPClientInfo cInfo,
												ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	json reply;
	
	
	if(path.size() == 2) {
		DeviceNameArgValidator vDevice;
		RelayStateArgValidator v1;
		
		auto deviceStr = path.at(1);
		
		if(vDevice.validateArg(deviceStr)){
			
			bool relayState = false;
			if(v1.getBoolFromJSON(JSON_ARG_STATE, url.body(), relayState)){
				
				transform(deviceStr.begin(), deviceStr.end(), deviceStr.begin(), ::tolower);
				
				bool queued = false;
				if(deviceStr == "door"){
					queued = coopMgr.setDoor(relayState,[=]( bool didSucceed){
						
						json reply;
						
						if(didSucceed){
							makeStatusJSON(reply,STATUS_OK);
							(completion) (reply, STATUS_OK);
						}
						else {
							makeStatusJSON(reply, STATUS_BAD_REQUEST, "Set Failed" );;
							(completion) (reply, STATUS_BAD_REQUEST);
						}
						
					});
					
				}
				else if(deviceStr == "light"){
					queued = coopMgr.setLight(relayState,[=]( bool didSucceed){
						json reply;
						
						if(didSucceed){
							makeStatusJSON(reply,STATUS_OK);
							(completion) (reply, STATUS_OK);
						}
						else {
							makeStatusJSON(reply, STATUS_BAD_REQUEST, "Set Failed" );;
							(completion) (reply, STATUS_BAD_REQUEST);
						}
						
					});
				}
				
				if(!queued) {
					makeStatusJSON(reply, STATUS_UNAVAILABLE, "Server is not running" );;
					(completion) (reply, STATUS_UNAVAILABLE);
					return false;
				}
				return true;
				
			}
		}
		
	}
	
	return false;
	
}

static void Devices_NounHandler(ServerCmdQueue* cmdQueue,
										  REST_URL url,
										  TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	json reply;
	
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	string noun;
	
	bool isValidURL = false;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
 
	switch(url.method()){
		case HTTP_GET:
 			isValidURL = Devices_NounHandler_GET(cmdQueue,url,cInfo, completion);
			break;
			
		case HTTP_PUT:
 			isValidURL = Devices_NounHandler_PUT(cmdQueue,url,cInfo, completion);
			break;
	
		case HTTP_DELETE:
//			isValidURL = Devices_NounHandler_DELETE(cmdQueue,url,cInfo, completion);
			break;

		case HTTP_PATCH:
//			isValidURL = Devices_NounHandler_PATCH(cmdQueue,url,cInfo, completion);
			break;
			
	 
		default:
			(completion) (reply, STATUS_INVALID_METHOD);
			return;
	}
	
	if(!isValidURL) {
		(completion) (reply, STATUS_NOT_FOUND);
	}
};


// MARK:  SCHEMA NOUN HANDLERS

static void Schema_NounHandler(ServerCmdQueue* cmdQueue,
										  REST_URL url,
										  TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	json reply;
	
	auto db = coopMgr.getDB();
	
	// CHECK METHOD
	if(url.method() != HTTP_GET ) {
		(completion) (reply, STATUS_INVALID_METHOD);
		return;
	}
	
	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	// CHECK sub paths
	if(noun != NOUN_SCHEMA){
		(completion) (reply, STATUS_NOT_FOUND);
		return;
	}
 
	reply[string(JSON_ARG_SCHEMA)] = db->schemaJSON();;
 
	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
};

// MARK:  VALUES NOUN HANDLERS

static void Values_Handler(ServerCmdQueue* cmdQueue,
										  REST_URL url,
										  TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	json reply;
	ServerCmdArgValidator v1;
	string str;
	
	auto db = coopMgr.getDB();
	
	// CHECK METHOD
	if(url.method() != HTTP_GET ) {
		(completion) (reply, STATUS_INVALID_METHOD);
		return;
	}
	
	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	// CHECK sub paths
	if(noun != NOUN_VALUES){
		(completion) (reply, STATUS_NOT_FOUND);
		return;
	}
 
	if(path.size() == 1) {
		eTag_t eTag = 0;
		
		if(v1.getStringFromMap("If-None-Match", url.headers(), str)){
			char* p;
			eTag = strtol(str.c_str(), &p, 0);
			if(*p != 0) eTag = 0;
		}
			
		reply[string(JSON_ARG_VALUES)] = db->currentValuesJSON(eTag);
		reply[string(JSON_ARG_ETAG)] = db->lastEtag();
		
		reply[string(JSON_ARG_STATE)] 		= coopMgr.coopState();
		reply[string(JSON_ARG_STATESTR)] 	= CoopMgrDevice::stateString(coopMgr.coopState());

		makeStatusJSON(reply,STATUS_OK);
		(completion) (reply, STATUS_OK);

	}
	else 	if(path.size() == 2) {
		
		string key = path.at(1);

		reply[string(JSON_ARG_VALUES)] = db->currentJSONForKey(key);
		makeStatusJSON(reply,STATUS_OK);
		(completion) (reply, STATUS_OK);
	}
};


// MARK: HISTORY NOUN HANDLER

static bool History_NounHandler_GET(ServerCmdQueue* cmdQueue,
											  REST_URL url,
											  TCPClientInfo cInfo,
											  ServerCmdQueue::cmdCallback_t completion) {
	using namespace rest;
	json reply;
	ServerCmdArgValidator v1;
	string str;

	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	auto db = coopMgr.getDB();

	// CHECK sub paths
	map<string ,string> propList;
	
	 if (path.size() == 2){
		
		string key = path.at(1);
		CoopMgrDB::historicValues_t history;
 
		 float days = 0;
		 int limit = 0;
	 
		 if(v1.getStringFromMap(JSON_ARG_LIMIT, url.headers(), str)){
			 char* p;
			 limit = (int) strtol(str.c_str(), &p, 10);
			 if(*p != 0) days = 0;
		 }

		 if(v1.getStringFromMap(JSON_ARG_DAYS, url.headers(), str)){
			 char* p;
			 days =  strtof(str.c_str(), &p);
			 if(*p != 0) days = 0;
		 }
		 
		 if(db->historyForKey(key, history, days, limit)){

			 json j;
			 for (auto& entry : history) {
			 
				 json j1;
				 j1[string(CoopMgrDB::JSON_ARG_VALUE)] 		=  entry.second;
				 j1[string(CoopMgrDB::JSON_ARG_TIME)] 		=   entry.first;
				 j.push_back(j1);
			 }

			 reply[string(JSON_ARG_VALUES)] = j;
		 }
		else {
			return false;
		}
	}
	else {
		return false;
	}
 
	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
	return true;
}

static bool History_NounHandler_DELETE(ServerCmdQueue* cmdQueue,
											  REST_URL url,
											  TCPClientInfo cInfo,
													ServerCmdQueue::cmdCallback_t completion) {
	using namespace rest;
	json reply;
	ServerCmdArgValidator v1;
	string str;
	
	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	auto db = coopMgr.getDB();
	
	// CHECK sub paths
	map<string ,string> propList;
	
	float days = 0;
	
	if(v1.getStringFromMap(JSON_ARG_DAYS, url.headers(), str)){
		char* p;
		days =  strtof(str.c_str(), &p);
		if(*p != 0) days = 0;
	}
	
	
	if (path.size() == 1){
		if(db->removeHistoryForKey(string(), days)){
			makeStatusJSON(reply,STATUS_NO_CONTENT);
			(completion) (reply, STATUS_NO_CONTENT);
			return true;
		}
	}
	else if (path.size() == 2){
		string key = path.at(1);
		if(db->removeHistoryForKey(key, days)){
			
			makeStatusJSON(reply,STATUS_NO_CONTENT);
			(completion) (reply, STATUS_NO_CONTENT);
			return true;
			
		}
		
	}
	return false;
}

static void History_NounHandler(ServerCmdQueue* cmdQueue,
										  REST_URL url,
										  TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {

	using namespace rest;
	json reply;
	
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	string noun;
	
	bool isValidURL = false;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}

	switch(url.method()){
		case HTTP_GET:
			isValidURL = History_NounHandler_GET(cmdQueue,url,cInfo, completion);
			break;
			
//		case HTTP_PUT:
//			isValidURL = History_NounHandler_PUT(cmdQueue,url,cInfo, completion);
//			break;

//		case HTTP_PATCH:
//			isValidURL = History_NounHandler_PATCH(cmdQueue,url,cInfo, completion);
//			break;

//		case HTTP_POST:
//			isValidURL = History_NounHandler_POST(cmdQueue,url,cInfo, completion);
//			break;
//
		case HTTP_DELETE:
			isValidURL = History_NounHandler_DELETE(cmdQueue,url,cInfo, completion);
			break;
  
		default:
			(completion) (reply, STATUS_INVALID_METHOD);
			return;
	}
	
	if(!isValidURL) {
		(completion) (reply, STATUS_NOT_FOUND);
	}
};

// MARK:  EVENTS NOUN HANDLER
 
static bool Events_NounHandler_GET(ServerCmdQueue* cmdQueue,
											  REST_URL url,
											  TCPClientInfo cInfo,
											  ServerCmdQueue::cmdCallback_t completion) {
	using namespace rest;
	json reply;
	ServerCmdArgValidator v1;
	string str;
	
	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	auto db = coopMgr.getDB();
	
	CoopMgrDB::historicEvents_t events;
	
	float days = 0;
	int limit = 0;
	
	if(v1.getStringFromMap(JSON_ARG_LIMIT, url.headers(), str)){
		char* p;
		limit = (int) strtol(str.c_str(), &p, 10);
		if(*p != 0) days = 0;
	}
	
	if(v1.getStringFromMap(JSON_ARG_DAYS, url.headers(), str)){
		char* p;
		days =  strtof(str.c_str(), &p);
		if(*p != 0) days = 0;
	}
	
	if(db->historyForEvents(events, days, limit)){
		
		json j;
		for (auto& entry : events) {
			
			json j1;
			j1[string(CoopMgrDB::JSON_ARG_EVENT)] 		=  entry.second;
			j1[string(CoopMgrDB::JSON_ARG_TIME)] 		=   entry.first;
			j.push_back(j1);
		}
		
		reply[string(JSON_ARG_EVENTS)] = j;
	}
	else {
		return false;
	}
	
	
	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
	return true;
}

static bool Events_NounHandler_DELETE(ServerCmdQueue* cmdQueue,
											  REST_URL url,
											  TCPClientInfo cInfo,
												  ServerCmdQueue::cmdCallback_t completion) {
	using namespace rest;
	json reply;
	ServerCmdArgValidator v1;
	string str;
	
	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	auto db = coopMgr.getDB();
	
	// CHECK sub paths
	float days = 0;
	
	if(v1.getStringFromMap(JSON_ARG_DAYS, url.headers(), str)){
		char* p;
		days =  strtof(str.c_str(), &p);
		if(*p != 0) days = 0;
	}
	
	
	if(db->removeHistoryForEvents(days)){
		makeStatusJSON(reply,STATUS_NO_CONTENT);
		(completion) (reply, STATUS_NO_CONTENT);
		return true;
	}
	
	return false;
}

static void Events_NounHandler(ServerCmdQueue* cmdQueue,
										  REST_URL url,
										  TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {

	using namespace rest;
	json reply;
	
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	string noun;
	
	bool isValidURL = false;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}

	switch(url.method()){
		case HTTP_GET:
			isValidURL = Events_NounHandler_GET(cmdQueue,url,cInfo, completion);
			break;
			
//		case HTTP_PUT:
//			isValidURL = History_NounHandler_PUT(cmdQueue,url,cInfo, completion);
//			break;

//		case HTTP_PATCH:
//			isValidURL = History_NounHandler_PATCH(cmdQueue,url,cInfo, completion);
//			break;

//		case HTTP_POST:
//			isValidURL = History_NounHandler_POST(cmdQueue,url,cInfo, completion);
//			break;
//
		case HTTP_DELETE:
			isValidURL = Events_NounHandler_DELETE(cmdQueue,url,cInfo, completion);
			break;
  
		default:
			(completion) (reply, STATUS_INVALID_METHOD);
			return;
	}
	
	if(!isValidURL) {
		(completion) (reply, STATUS_NOT_FOUND);
	}
};

// MARK:  PROPERTIES NOUN HANDLER

static bool Properties_NounHandler_GET(ServerCmdQueue* cmdQueue,
											  REST_URL url,
											  TCPClientInfo cInfo,
											  ServerCmdQueue::cmdCallback_t completion) {
	using namespace rest;
	json reply;

	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	auto db = coopMgr.getDB();

	// CHECK sub paths
	map<string ,string> propList;
	
	if(path.size() == 1){
		propList = db->getProperties();
		
	}else if (path.size() == 2){
		
		string propName = path.at(1);
		string value;
		
		if(db->getProperty(propName, &value)){
			propList[propName] = value;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
 
	json propEntries;

	for(const auto& [key, value] : propList) {
		
		// never list these
		if(key == CoopMgrDB::PROP_API_SECRET
			|| key == CoopMgrDB::PROP_API_KEY )
			continue;
		propEntries[key] = value;
	}

	reply[ string(JSON_ARG_PROPERTIES) ] = propEntries;
	
	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
	return true;
}


static bool Properties_NounHandler_PATCH(ServerCmdQueue* cmdQueue,
											  REST_URL url,
											  TCPClientInfo cInfo,
											  ServerCmdQueue::cmdCallback_t completion) {
	using namespace rest;
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	auto body 	= url.body();
	
	json reply;
	
	auto db = coopMgr.getDB();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	bool didUpdate = false;


	for(auto it =  body.begin(); it != body.end(); ++it) {
		string key = Utils::trim(it.key());

		if(body[it.key()].is_string()){
			string value = Utils::trim(it.value());
			
			if(db->setProperty(key, value)){
				didUpdate = true;
			}
		}
		else if(body[it.key()].is_number()){
			string value =to_string(it.value());

			if(db->setProperty(key, value)){
				didUpdate = true;
			}
		}
		else if(body[it.key()].is_boolean()){
			string value =  it.value()?"1":"0";
			
			if(db->setProperty(key, value)){
				didUpdate = true;
			}
			
		} else if(body[it.key()].is_null()){
			// delete property
			if(db->removeProperty( key)){
				didUpdate = true;
			}
		}
	}
	
	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
	return true;
}


static bool Properties_NounHandler_DELETE(ServerCmdQueue* cmdQueue,
											  REST_URL url,
											  TCPClientInfo cInfo,
														ServerCmdQueue::cmdCallback_t completion) {
	using namespace rest;
	json reply;
	
	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}

	auto db = coopMgr.getDB();

	if (path.size() == 2){
		string propName = path.at(1);
		
		if(!db->removeProperty(propName)){
				return false;
		}
		
	}
	else {
		return false;
	}
	
	// CHECK sub paths
	
	makeStatusJSON(reply,STATUS_NO_CONTENT);
	(completion) (reply, STATUS_NO_CONTENT);
	return true;
}




static void Properties_NounHandler(ServerCmdQueue* cmdQueue,
										  REST_URL url,
										  TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {

	using namespace rest;
	json reply;
	
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	string noun;
	
	bool isValidURL = false;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}

	switch(url.method()){
		case HTTP_GET:
			isValidURL = Properties_NounHandler_GET(cmdQueue,url,cInfo, completion);
			break;
			
//		case HTTP_PUT:
//			isValidURL = Properties_NounHandler_PUT(cmdQueue,url,cInfo, completion);
//			break;

		case HTTP_PATCH:
			isValidURL = Properties_NounHandler_PATCH(cmdQueue,url,cInfo, completion);
			break;

//		case HTTP_POST:
//			isValidURL = Properties_NounHandler_POST(cmdQueue,url,cInfo, completion);
//			break;
//
		case HTTP_DELETE:
			isValidURL = Properties_NounHandler_DELETE(cmdQueue,url,cInfo, completion);
			break;
  
		default:
			(completion) (reply, STATUS_INVALID_METHOD);
			return;
	}
	
	if(!isValidURL) {
		(completion) (reply, STATUS_NOT_FOUND);
	}
};

// MARK:  STATE NOUN HANDLERS

static bool State_NounHandler_GET(ServerCmdQueue* cmdQueue,
											  REST_URL url,
											  TCPClientInfo cInfo,
											  ServerCmdQueue::cmdCallback_t completion) {
	using namespace rest;
	using namespace timestamp;

	json reply;
	
	auto path = url.path();
	string noun;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	// CHECK sub paths
	if(noun != NOUN_STATE){
		(completion) (reply, STATUS_NOT_FOUND);
		return false;
	}
	 
	reply[string(JSON_ARG_DATE)] = TimeStamp().RFC1123String();
	reply[string(JSON_ARG_UPTIME)]	= coopMgr.upTime();

	reply[string(JSON_ARG_VERSION)] = CoopMgr::CoopMgr_Version;
	reply[string(JSON_ARG_BUILD_TIME)]	=  string(__DATE__) + " " + string(__TIME__);

	reply[string(JSON_ARG_STATE)] 		= coopMgr.coopState();
	reply[string(JSON_ARG_STATESTR)] 	= CoopMgrDevice::stateString(coopMgr.coopState());

	reply[string(JSON_ARG_TEMP_SENSOR1)]	= coopMgr.tempSensor1State();
	reply[string(JSON_ARG_COOP_DEVICE)]		= coopMgr.coopState();
	
	solarTimes_t solar;
	if( coopMgr.getSolarEvents(solar)) {
		reply["civilSunRise"] = solar.civilSunRiseMins;
		reply["sunRise"] = solar.sunriseMins;
		reply["sunSet"] = solar.sunSetMins;
		reply["civilSunSet"] = solar.civilSunSetMins;
		reply["latitude"] = solar.latitude;
		reply["longitude"] = solar.longitude;
		reply["gmtOffset"] = solar.gmtOffset;
		reply["timeZone"] = solar.timeZoneString;
		reply["midnight"] = solar.previousMidnight;
	}

 
	double temp;
	if(getCPUTemp(temp)){
		reply[string(JSON_ARG_CPU_TEMP)] =   temp;
	}

	struct utsname buffer;
	if (uname(&buffer) == 0){
		reply[string(JSON_ARG_OS_SYSNAME)] =   string(buffer.sysname);
		reply[string(JSON_ARG_OS_NODENAME)] =   string(buffer.nodename);
		reply[string(JSON_ARG_OS_RELEASE)] =   string(buffer.release);
		reply[string(JSON_ARG_OS_VERSION)] =   string(buffer.version);
		reply[string(JSON_ARG_OS_MACHINE)] =   string(buffer.machine);
	}
	
	makeStatusJSON(reply,STATUS_OK);
	(completion) (reply, STATUS_OK);
	return true;
}


static bool State_NounHandler_PUT(ServerCmdQueue* cmdQueue,
											  REST_URL url,
											  TCPClientInfo cInfo,
											 ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	
	ServerCmdArgValidator v1;
	//	auto db = coopMgr.getDB();
	
	string noun;
	json reply;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	if(path.size() == 2) {
		
		string subpath =   path.at(1);
		string str;
 
	}
	
	
	return false;
}


static void State_NounHandler(ServerCmdQueue* cmdQueue,
										  REST_URL url,
										  TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {

	using namespace rest;
	json reply;
	
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	string noun;
	
	bool isValidURL = false;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}

	switch(url.method()){
		case HTTP_GET:
			isValidURL = State_NounHandler_GET(cmdQueue,url,cInfo, completion);
			break;
			
		case HTTP_PUT:
			isValidURL = State_NounHandler_PUT(cmdQueue,url,cInfo, completion);
			break;

//		case HTTP_PATCH:
//			isValidURL = State_NounHandler_PATCH(cmdQueue,url,cInfo, completion);
//			break;
//
//		case HTTP_POST:
//			isValidURL = State_NounHandler_POST(cmdQueue,url,cInfo, completion);
//			break;
//
//		case HTTP_DELETE:
//			isValidURL = Events_NounHandler_DELETE(cmdQueue,url,cInfo, completion);
//			break;
  
		default:
			(completion) (reply, STATUS_INVALID_METHOD);
			return;
	}
	
	if(!isValidURL) {
		(completion) (reply, STATUS_NOT_FOUND);
	}
};

// MARK: LOG - NOUN HANDLER

static bool Log_NounHandler_GET(ServerCmdQueue* cmdQueue,
											  REST_URL url,
											  TCPClientInfo cInfo,
											  ServerCmdQueue::cmdCallback_t completion) {
	using namespace rest;

	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	json reply;
	auto db = coopMgr.getDB();

	string subpath;
	
	if(path.size() > 1){
		subpath =   path.at(1);
	}

	// GET /log
	if(subpath == SUBPATH_STATE) {
  
		reply[string(JSON_ARG_LOGFLAGS)] =  LogMgr::shared()->_logFlags;
 
		makeStatusJSON(reply,STATUS_OK);
		(completion) (reply, STATUS_OK);
		return true;
	}
	else if(subpath == SUBPATH_FILE) {
		
		string path;
		
		db->getProperty(string(CoopMgrDB::PROP_LOG_FILE), &path);
		reply[string(JSON_ARG_FILEPATH)] =  path;
	
		makeStatusJSON(reply,STATUS_OK);
		(completion) (reply, STATUS_OK);
		return true;
	}
		
	  return false;
}

static bool Log_NounHandler_PUT(ServerCmdQueue* cmdQueue,
												REST_URL url,
												TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	
	ServerCmdArgValidator v1;
 
	json reply;
	string subpath;

	auto db = coopMgr.getDB();

	if(path.size() > 1){
		subpath =   path.at(1);
	}

	if(subpath == SUBPATH_STATE) {
		uint8_t logFlags;
	
		if(v1.getByteFromJSON(JSON_ARG_LOGFLAGS, url.body(), logFlags)){
			
			LogMgr::shared()->_logFlags = logFlags;
			db->setProperty(string(CoopMgrDB::PROP_LOG_FLAGS),  to_hex <unsigned char>(logFlags,true));

			makeStatusJSON(reply,STATUS_OK);
			(completion) (reply, STATUS_OK);
			return true;

		}
	}
	else if(subpath == SUBPATH_FILE) {
		
		string path;
		
		if(v1.getStringFromJSON(JSON_ARG_FILEPATH, url.body(), path)){
			// set the log file
			
			bool success = LogMgr::shared()->setLogFilePath(path);
			if(success){
				db->setProperty(string(CoopMgrDB::PROP_LOG_FILE), path);
 
				makeStatusJSON(reply,STATUS_OK);
				(completion) (reply, STATUS_OK);
			}
			else {
				string lastError =  string("Error: ") + to_string(errno);
				string lastErrorString = string(::strerror(errno));
				
				makeStatusJSON(reply, STATUS_BAD_REQUEST, lastError, lastErrorString);;
				(completion) (reply, STATUS_BAD_REQUEST);
			}
			return true;
		}
	}
 
	return false;
}

static bool Log_NounHandler_PATCH(ServerCmdQueue* cmdQueue,
												REST_URL url,
												TCPClientInfo cInfo,
										  ServerCmdQueue::cmdCallback_t completion) {
	
	using namespace rest;
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	
	ServerCmdArgValidator v1;
 
	json reply;
 
	string str;
	
	if(v1.getStringFromJSON(JSON_ARG_MESSAGE, url.body(), str)){
		
		LogMgr::shared()->logTimedStampString(str);
		makeStatusJSON(reply,STATUS_OK);
		(completion) (reply, STATUS_OK);
		return true;
	}

	return false;
}

static void Log_NounHandler(ServerCmdQueue* cmdQueue,
										 REST_URL url,  // entire request
										 TCPClientInfo cInfo,
										 ServerCmdQueue::cmdCallback_t completion) {
	using namespace rest;
	json reply;
	
	auto path = url.path();
	auto queries = url.queries();
	auto headers = url.headers();
	string noun;
	
	bool isValidURL = false;
	
	if(path.size() > 0) {
		noun = path.at(0);
	}
	
	switch(url.method()){
		case HTTP_GET:
			isValidURL = Log_NounHandler_GET(cmdQueue,url,cInfo, completion);
			break;

		case HTTP_PUT:
			isValidURL = Log_NounHandler_PUT(cmdQueue,url,cInfo, completion);
			break;

		case HTTP_PATCH:
			isValidURL = Log_NounHandler_PATCH(cmdQueue,url,cInfo, completion);
			break;

//		case HTTP_POST:
//			isValidURL = Log_NounHandler_POST(cmdQueue,url,cInfo, completion);
//			break;
//
//		case HTTP_DELETE:
//			isValidURL = Log_NounHandler_DELETE(cmdQueue,url,cInfo, completion);
//			break;
  
		default:
			(completion) (reply, STATUS_INVALID_METHOD);
			return;
	}
	
	if(!isValidURL) {
		(completion) (reply, STATUS_NOT_FOUND);
	}
}

// MARK: -  register server nouns

void registerServerNouns() {
	// create the server command processor
	auto cmdQueue = ServerCmdQueue::shared();

	cmdQueue->registerNoun(NOUN_STATE, 	State_NounHandler);
	cmdQueue->registerNoun(NOUN_SCHEMA, 	Schema_NounHandler);
	cmdQueue->registerNoun(NOUN_VALUES, 	Values_Handler);
	cmdQueue->registerNoun(NOUN_DEVICES, 	Devices_NounHandler);

	cmdQueue->registerNoun(NOUN_PROPERTIES, Properties_NounHandler);
	cmdQueue->registerNoun(NOUN_LOG, 		Log_NounHandler);
	cmdQueue->registerNoun(NOUN_HISTORY,	 History_NounHandler);
	cmdQueue->registerNoun(NOUN_EVENTS, 	Events_NounHandler);
}


