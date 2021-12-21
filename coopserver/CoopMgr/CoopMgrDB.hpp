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
#include<functional>
#include <map>
#include <string>
#include "json.hpp"
#include <sqlite3.h>
 
#include "CoopMgrCommon.h"

using namespace std;
using namespace nlohmann;

typedef  unsigned long eTag_t;

class CoopMgrDB {
 
public:

	
	constexpr static string_view PROP_TRIGGER_PREFIX 			= "trigger-";

	constexpr static string_view PROP_LOG_FLAGS					= "log-flags";
	constexpr static string_view PROP_LOG_FILE					= "log-filepath";
	constexpr static string_view PROP_API_KEY						= "api-key";
	constexpr static string_view PROP_API_SECRET					= "api-secret";
	constexpr static string_view PROP_LATLONG						= "lat-long";

	constexpr static string_view PROP_TELNET_PORT				= "telnet-port";
	constexpr static string_view PROP_REST_PORT					= "rest-port";
	constexpr static string_view PROP_ALLOW_REMOTE_TELNET		= "allow-remote-telnet";

	 
	
	typedef enum {
		INVALID = 0,
		BOOL,				// Bool ON/OFF
		INT,				// Int
		SECONDS,			// sec
		MINUTES,			// mins
		DEGREES_C,		// degC
		BINARY,			// Binary 8 bits 000001
		STRING,			// string
		IGNORE,
		UNKNOWN,
	}valueSchemaUnits_t;
	
	typedef enum {
		TR_IGNORE 	= 0,
		TR_STATIC	= 1,
		TR_TRACK		= 2,
		TR_DONT_TRACK =  3 // use latest value, dont track
	}valueTracking_t;

	// dont change these numbers, they persist in database
	typedef enum {
		EV_UNKNOWN 				= 0,
		EV_START					= 1,
		EV_SHUTDOWN				= 2,
		
	}ph_event_t;

private:
	
	typedef struct {
	  string  					description;
	  valueSchemaUnits_t  		units;
	  valueTracking_t			tracking;
  } valueSchema_t;

public:
	constexpr static string_view JSON_ARG_NAME			= "name";
	constexpr static string_view JSON_ARG_TRACKING		= "tracking";
	constexpr static string_view JSON_ARG_UNITS			= "units";
	constexpr static string_view JSON_ARG_SUFFIX			= "suffix";
	constexpr static string_view JSON_ARG_VALUE			= "value";
	constexpr static string_view JSON_ARG_EVENT			= "event";
	constexpr static string_view JSON_ARG_TIME			= "time";
	constexpr static string_view JSON_ARG_DISPLAYSTR		= "display";
	
	typedef vector<pair<time_t, string>> historicValues_t;
	typedef vector<pair<time_t, ph_event_t>> historicEvents_t;

	CoopMgrDB();
  ~CoopMgrDB();
	bool initSchemaFromFile(string filePath);
	bool initLogDatabase(string filePath);

	void clear();
	
	bool insertValue(string key, string value, time_t when,  eTag_t eTag);
	bool insertValues(map<string,string>  values, time_t when = 0);

	void addSchema(string key,  valueSchemaUnits_t units, string description, valueTracking_t tracking);

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


	// MARK: - Events
	bool logEvent(ph_event_t evt, time_t when = 0);
	bool historyForEvents( historicEvents_t &events, float days = 0.0, int limit = 0);
	bool removeHistoryForEvents(float days);
	string displayStringForEvent(ph_event_t evt);

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

 
private:
	
	
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
	bool		insertValueToDB(string key, string value, time_t time );
	bool		saveUniqueValueToDB(string key, string value, time_t time );
		map<string, pair<time_t, string>> _values;
 
	map<string,valueSchemaUnits_t>  _schemaMap;
	map<string, valueSchema_t>	_schema;
	map<string, eTag_t> 			_etagMap;
	eTag_t							_eTag;		// last change tag

	map<string,string> 			_properties;
	string 						_propertyFilePath;

	double		_longitude;
	double 		_latitude;


};
#endif /* CoopMgrDB_hpp */
