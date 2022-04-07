//
//  CoopMgrDB.hpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/20/21.
//

#ifndef CoopMgrDB_hpp
#define CoopMgrDB_hpp
 
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <mutex>
#include <random>
#include <set>
#include <vector>
#include <tuple>
#include <functional>
#include <map>
#include <string>
#include "json.hpp"
#include <sqlite3.h>
 
#include "CoopMgrCommon.h"
#include "Action.hpp"
#include "Event.hpp"
#include "ErrorMgr.hpp"

using namespace std;
using namespace nlohmann;

typedef uint64_t eTag_t;
#define MAX_ETAG UINT64_MAX

typedef  unsigned short eventGroupID_t;
bool str_to_EventGroupID(const char* str, eventGroupID_t *eventGroupIDOut = NULL);
string  EventGroupID_to_string(eventGroupID_t eventGroupID);


class CoopMgrDB {
 
public:
	
	constexpr static string_view PROP_LOG_FLAGS					= "log-flags";
	constexpr static string_view PROP_LOG_FILE					= "log-filepath";
	constexpr static string_view PROP_API_KEY						= "api-key";
	constexpr static string_view PROP_API_SECRET					= "api-secret";
	constexpr static string_view PROP_LATLONG						= "lat-long";

	constexpr static string_view PROP_TELNET_PORT				= "telnet-port";
	constexpr static string_view PROP_REST_PORT					= "rest-port";
	constexpr static string_view PROP_ALLOW_REMOTE_TELNET		= "allow-remote-telnet";
	constexpr static string_view PROP_EVENT		= "events";	// event data follows
	constexpr static string_view PROP_EVENT_GROUPS		= "event.groups";	// event data follows
	constexpr static string_view PROP_ARG_EVENT_GROUPID		= "groupID";	// event data follows
	constexpr static string_view PROP_ARG_EVENTIDS				= "eventIDs";	// event data follows
 
	constexpr static string_view SCHEMA_TAG_ERRCOUNT		= "ERR_COUNT";

	typedef enum {
		INVALID = 0,
		BOOL,				// Bool ON/OFF
		INT,				// Int
		MAH,				// mAh milliAmp hours
		PERMILLE, 		// (per thousand) sign ‰
		PERCENT, 		// (per hundred) sign ‰
		DEKAWATTHOUR,	 // .01kWh
		WATTS, 			// W
		MILLIVOLTS,		// mV
		MILLIAMPS,		// mA
		SECONDS,			// sec
		MINUTES,			// mins
		DEGREES_C,		// degC
		VOLTS,			// V
		HERTZ,			// Hz
		AMPS,				// A
		BINARY,			// Binary 8 bits 000001
		VE_PRODUCT,		// VE.PART
		STRING,			// string
		IGNORE,
		DOOR_STATE,
		ON_OFF,
		UNKNOWN,
	}valueSchemaUnits_t;
	
	typedef enum {
		TR_IGNORE 	= 0,
		TR_STATIC	= 1,
		TR_TRACK		= 2,
		TR_DONT_TRACK =  3 // use latest value, dont track
	}valueTracking_t;

	// historical event
	// dont change these numbers, they persist in database
	typedef enum {
		EV_UNKNOWN 				= 0,
		EV_START					= 1,
		EV_SHUTDOWN				= 2,
		
	}h_event_t;

private:
	
	typedef struct {
	  string  					description;
	  valueSchemaUnits_t  	units;
	  valueTracking_t			tracking;
		double					theshold;	// used by valueShouldUpdate for  database diff. , minus == default
  } valueSchema_t;

public:
	constexpr static string_view JSON_ARG_NAME			= "name";
	constexpr static string_view JSON_ARG_TRACKING		= "tracking";
	constexpr static string_view JSON_ARG_UNITS			= "units";
	constexpr static string_view JSON_ARG_SUFFIX			= "suffix";
	constexpr static string_view JSON_ARG_VALUE			= "value";
	constexpr static string_view JSON_ARG_HISTORICAL_EVENT			= "historicalevent";
	constexpr static string_view JSON_ARG_TIME			= "time";
	constexpr static string_view JSON_ARG_DISPLAYSTR		= "display";
	constexpr static string_view JSON_ARG_ETAG			= "ETag";
	
	
	constexpr static string_view JSON_ARG_FACILITY			= "err.facility";
	constexpr static string_view JSON_ARG_LEVEL				= "err.level";
	constexpr static string_view JSON_ARG_DEVICE				= "err.device";
	constexpr static string_view JSON_ARG_MESSAGE			= "err.message";
	constexpr static string_view JSON_ARG_ERROR				= "err.error";
 
	typedef vector<pair<time_t, string>> historicValues_t;
	typedef vector<pair<time_t, h_event_t>> historicEvents_t;
	
	typedef vector<tuple<eTag_t, 	// eTag
								time_t,	// time
				ErrorMgr::level_t,	// level
			ErrorMgr::facility_t,	// facility
							uint8_t,	 // deviceID
							string, // message
							string  // error
							>> errorLogValues_t;

	CoopMgrDB();
  ~CoopMgrDB();
	bool initSchemaFromFile(string filePath);
	bool initLogDatabase(string filePath);
 
	void clear();
	
	bool insertValue(string key, string value, time_t when,  eTag_t eTag);
	bool insertValues(map<string,string>  values, time_t when = 0);

	void addSchema(string key,
							valueSchemaUnits_t units,
							 double threshold,
							 string description,
							 valueTracking_t tracking);

	vector<string> keysChangedSinceEtag( eTag_t eTag);

	string displayStringForValue(string key, string value);
	void dumpMap();

	// MARK: -  API Secrets
	bool apiSecretCreate(string APIkey, string APISecret);
	bool apiSecretDelete(string APIkey);
	bool apiSecretSetSecret(string APIkey, string APISecret);
	bool apiSecretGetSecret(string APIkey, string &APISecret);
	bool apiSecretMustAuthenticate();

	// MARK: -   Data
	json	schemaJSON();
	json	currentValuesJSON(eTag_t  eTag = 0);
	json  jsonForValue(string key, string value);
	json	currentJSONForKey(string key);
	eTag_t lastEtag() { return  _eTag;};

	bool	historyForKey(string key, historicValues_t &values, float days = 0.0, int limit = 0);
	bool 	removeHistoryForKey(string key, float days);

	// MARK: - History
	bool logHistoricalEvent(h_event_t evt, time_t when = 0);
	bool historyForEvents( historicEvents_t &events, float days = 0.0, int limit = 0);
	bool trimHistory(float days);
	string displayStringForHistoricalEvent(h_event_t evt);

	// MARK: - properties
	bool savePropertiesToFile(string filePath = "") ;
	bool restorePropertiesFromFile(string filePath = "");
 
	bool setProperty(string key, string value);
	bool getProperty(string key, string *value);
	bool setPropertyIfNone(string key, string value);

	bool getUint16Property(string key, uint16_t * value);
	bool getFloatProperty(string key, float * valOut);
	bool getBoolProperty(string key, bool * valOut);

	bool removeProperty(string key);
	map<string ,string> getProperties();

	// MARK: -   events
	bool eventsIsValid(eventID_t eventID);
	bool eventSave(Event event, eventID_t* eventIDOut = NULL);
	bool eventFind(string name, eventID_t* eventID);
	bool eventDelete(eventID_t eventID);
	bool eventUpdate(eventID_t eventID, Event event);

	bool eventSetName(eventID_t eventID, string name);
	string eventGetName(eventID_t eventID);
	optional<reference_wrapper<Event>> eventsGetEvent(eventID_t eventID);
	vector<eventID_t> allEventsIDs();
//	vector<eventID_t> matchingEventIDs(EventTrigger trig);
	vector<eventID_t> eventsMatchingAppEvent(EventTrigger::app_event_t appEvent);
	vector<eventID_t> eventsThatNeedToRun(solarTimes_t &solar, time_t localNow);
	vector<eventID_t> eventsInTheFuture(solarTimes_t &solar, time_t localNow);
	bool eventSetLastRunTime(eventID_t eventID, time_t localNow);
	void resetAllEventsLastRun();

	// MARK: -  event groups
	bool eventGroupIsValid(eventGroupID_t eventGroupID);
	bool eventGroupCreate(eventGroupID_t* eventGroupID, const string name);
	bool eventGroupDelete(eventGroupID_t eventGroupID);
	bool eventGroupFind(string name, eventGroupID_t* eventGroupID);
	bool eventGroupSetName(eventGroupID_t eventGroupID, string name);
	string eventGroupGetName(eventGroupID_t eventGroupID);
	bool eventGroupAddEvent(eventGroupID_t eventGroupID,  eventID_t eventID);
	bool eventGroupRemoveEvent(eventGroupID_t eventGroupID, eventID_t eventID);
	bool eventGroupContainsEventID(eventGroupID_t eventGroupID, eventID_t eventID);
	vector<eventID_t> eventGroupGetEventIDs(eventGroupID_t eventGroupID);
	vector<eventGroupID_t> allEventGroupIDs();
	void reconcileEventGroups(const solarTimes_t &solar, time_t localNow);

	// MARK: -  error logging into database
	
	void logError(ErrorMgr::level_t level,
				ErrorMgr::facility_t 	facility,
				uint8_t 		deviceID,
				string message,
				 string error = "");

	bool historyForErrors(errorLogValues_t &values, eTag_t &eTagOut,
								 eTag_t eTag,  float days = 0.0, int limit = 0);

	bool getErrorCount(eTag_t eTag, int &countOut);
 
	
	bool getErrorLogEtag(eTag_t &eTagOut);

	bool getLastErrorTime(time_t &whenOut);
	
	bool trimHistoryForErrorsByDays(float days);
	bool trimHistoryForErrorsByEtag(eTag_t eTag);

	// MARK: -
	
private:
	bool 		_isSetup;

	sqlite3 	*_sdb;
	mutable std::mutex _mutex;

	string defaultPropertyFilePath();
	bool 	refreshSolarEvents();

	valueSchema_t schemaForKey(string key);
	valueSchemaUnits_t unitsForKey(string key);
	string   unitSuffixForKey(string key);

	double 	normalizedDoubleForValue(string key, string value);
	int 		intForValue(string key, string value);
	
	bool 		valueShouldUpdate(string key, string value);

	bool		restoreValuesFromDB();
	bool 		restoreEventGroupFromJSON(json j);
	bool 		saveEventGroupToJSON(eventGroupID_t, json &j );

	bool		insertValueToDB(string key, string value, time_t time );
	bool		saveUniqueValueToDB(string key, string value, time_t time );

	
	map<string, pair<time_t, string>> _values;
	map<string,valueSchemaUnits_t>  _schemaMap;
	map<string, valueSchema_t>	_schema;
	map<eventID_t, Event> 		_events;
	
	typedef struct {
		string name;
		set<eventID_t>  eventIDs;
	} eventGroupInfo_t;

	map<eventGroupID_t, eventGroupInfo_t> 		_eventsGroups;

	map<string, eTag_t> 			_etagMap;
	eTag_t							_eTag;		// last change tag
	
	eTag_t							_cachedErrorTag;

	map<string,string> 			_properties;
	string 						_propertyFilePath;
  
	mt19937						_rng;

};
#endif /* CoopMgrDB_hpp */
