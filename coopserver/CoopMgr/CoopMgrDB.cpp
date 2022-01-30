//
//  CoopMgrDB.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/20/21.
//

#include "CoopMgrDB.hpp"
  
#include "TimeStamp.hpp"
#include "LogMgr.hpp"
#include "SolarTimeMgr.hpp"

#include "Utils.hpp"
#include <stdlib.h>
#include <algorithm> 
#include <regex>
#include <limits.h>

using namespace timestamp;

CoopMgrDB::CoopMgrDB(){
	
	_values.clear();
	_schema.clear();
	_events.clear();
	_eTag = 1;
	_etagMap.clear();
	_properties.clear();
	_sdb = NULL;
 

	// create RNG engine
	constexpr std::size_t SEED_LENGTH = 8;
  std::array<uint_fast32_t, SEED_LENGTH> random_data;
  std::random_device random_source;
  std::generate(random_data.begin(), random_data.end(), std::ref(random_source));
  std::seed_seq seed_seq(random_data.begin(), random_data.end());
	_rng =  std::mt19937{ seed_seq };

	_schemaMap = {
		{"Bool", BOOL},				// Bool ON/OFF
		{"Int", INT},				// Int
		{"mAh", MAH},				// mAh milliAmp hours
		{"‰", PERMILLE} ,			// (per thousand) sign ‰
		{"%", PERCENT} ,			// (per hundred) sign PERCENT
		{".01kWh", DEKAWATTHOUR},		// .01kWh
		{"W", WATTS}, 				// W
		{"mA",MILLIAMPS},			// mA
		{"mV", MILLIVOLTS},			// mV
		{"sec", SECONDS},			// sec
		{"mins",MINUTES},			// mins
		{"degC", DEGREES_C},		// degC
		{"V", VOLTS},				// V
		{"Hz", HERTZ},				// Hz
		{"A", AMPS},					// A
		{"Binary", BINARY},			// Binary 8 bits 000001
		{"VE.PART", VE_PRODUCT},		// VE.PART
		{"string", STRING},				// string
		{"ignore", IGNORE},				// ignore
		{"doorstate", DOOR_STATE},				// door state
		{"on-off", ON_OFF},				// door state
		
	};
  
 }

CoopMgrDB::~CoopMgrDB(){
	
	if(_sdb)
	{
		sqlite3_close(_sdb);
		_sdb = NULL;
	}
}


void CoopMgrDB::clear(){
	_values.clear();

}


bool CoopMgrDB::insertValues(map<string,string>  values, time_t when){
	
	bool didUpdate = false;
	if(when == 0)
		when = time(NULL);
	
	for (auto& [key, value] : values) {
		if(insertValue(key, value, when, _eTag )){
			didUpdate = true;
		}
	}
	
	if(didUpdate)
		_eTag++;
	
	return didUpdate;
	
};

bool CoopMgrDB::insertValue(string key, string value, time_t when, eTag_t eTag){

	bool updated = false;

	if(when == 0)
		when = time(NULL);
	
	valueSchema_t schema = schemaForKey(key);
	if(schema.tracking == TR_IGNORE){
		return false;
	}
	else if(schema.tracking == TR_STATIC) {
		//  keep last value
			_values[key] = make_pair(when, value) ;
		//  but dont put this in the database
	}
	else if(schema.tracking == TR_DONT_TRACK)
	{
	// only keep last value
		_values[key] = make_pair(when, value) ;
		
		 //  Add these to DB - but dont insert.. just update.
		saveUniqueValueToDB(key, value, when);
	}
	else if(schema.tracking == TR_TRACK)
	{
		if(valueShouldUpdate(key,value)){
			
			// update value
			_values[key] = make_pair(when, value);

			// record in DB
			insertValueToDB(key, value, when);
			updated = true;
			}
		}
	
	if(updated){
		_etagMap[key] = eTag;
	}

 //		printf("%s %s: %s \n", updated?"T":"F",  key.c_str(), value.c_str());

	return updated;
}


vector<string> CoopMgrDB::keysChangedSinceEtag( eTag_t tag){
	vector<string> changeList;
	changeList.clear();
	
	for (auto& [key, t] : _etagMap) {
		
		if(tag <= t){
			changeList.push_back(key);
		}
	}

	return changeList;
}

bool CoopMgrDB::valueShouldUpdate(string key, string value){
	
	bool shouldInsert = true;
	double triggerDiff = 0;
	
	
	valueSchema_t schema = schemaForKey(key);
	if(schema.tracking == TR_IGNORE)
		return false;
	
	if(_values.count(key)){
		auto lastpair = _values[key];
		valueSchema_t vs = _schema[key];
		
		// do we ignore it
		if(vs.units == IGNORE)
			return false;
		
		// quick string compare to see if its the same
		if(lastpair.second == value)
			return false;
		
		// see if it's a number
		char *p1, *p;
		double newVal = strtod(value.c_str(), &p);
		double oldval = strtod(lastpair.second.c_str(), &p1);
		if(*p == 0 && *p1 == 0 ){
			
			double diff = abs(oldval-newVal);
			
			switch (vs.units) {
				case DEGREES_C:
					triggerDiff = 1.0;
					break;
					
				case MILLIVOLTS:
				case MILLIAMPS:
				case MAH:
					triggerDiff = 500;
					break;
					
				case WATTS:
					triggerDiff = 5;
					break;
					
				case VOLTS:
					triggerDiff = 0.5;
					break;
					
				case AMPS:
					triggerDiff = 1.0;
					break;
					
				case PERMILLE:
					triggerDiff = 10;
					break;
					
				case PERCENT:
					triggerDiff = 1;
					break;
					
				case HERTZ:
					triggerDiff = 10;
					break;
					
				case DOOR_STATE:
					triggerDiff = 0;
					break;
					
				case ON_OFF:
					triggerDiff = 0;
					break;
					
				default:
					triggerDiff = 0;
					break;
			}
			
			// override trigger
			if(vs.theshold >= 0) {
				triggerDiff = vs.theshold;
			}
			shouldInsert = diff > triggerDiff;
		}
	}
	
	return shouldInsert;
}


CoopMgrDB::valueSchema_t CoopMgrDB::schemaForKey(string key){
	
	valueSchema_t schema = {"",UNKNOWN,TR_IGNORE};
 
	if(_schema.count(key)){
		schema =  _schema[key];
	}
	
	return schema;
}

CoopMgrDB::valueSchemaUnits_t CoopMgrDB::unitsForKey(string key){
	valueSchema_t schema = schemaForKey(key);
	return schema.units;
}

string   CoopMgrDB::unitSuffixForKey(string key){
	string suffix = {};
	
	switch(unitsForKey(key)){
			
		case MILLIVOLTS:
		case VOLTS:
			suffix = "V";
			break;

		case DOOR_STATE:
		case ON_OFF:
			break;
			
		case MILLIAMPS:
		case AMPS:
			suffix = "A";
			break;

		case MAH:
			suffix = "Ahrs";
			break;
 
		case DEKAWATTHOUR:
			suffix = "kWh";
			break;
  
		case DEGREES_C:
			suffix = "ºC";
			break;
 
		case PERMILLE:
		case PERCENT:
			suffix = "%";
			break;
			
		case WATTS:
			suffix = "W";
			break;

		case SECONDS:
			suffix = "Seconds";
			break;
			
		case MINUTES:
			suffix = "Minutes";
			break;

		case HERTZ:
			suffix = "Hz";
			break;
 
		default:
			break;
	}
	
	return suffix;
}

double CoopMgrDB::normalizedDoubleForValue(string key, string value){
	
	double retVal = 0;
	
	// see if it's a number
	char   *p;
	double val = strtod(value.c_str(), &p);
	if(*p == 0) {
 
		// normalize number
		
		switch(unitsForKey(key)){
				
			case DEGREES_C:
			case SECONDS:
			case MINUTES:
				retVal = val;
				break;
				
			default:
				break;
		}
	}
	return retVal;
}
 int CoopMgrDB::intForValue(string key, string value){
	
	int retVal = 0;
	
	 switch(unitsForKey(key)){

		 case MINUTES:
		 case SECONDS:
		 case INT:
		 {
			 int intval = 0;

			 if(sscanf(value.c_str(), "%d", &intval) == 1){
				retVal = intval;
			}
		 }
			 break;
			 
			 default:
			 break;
	 }
	  
	return retVal;
}



string CoopMgrDB::displayStringForValue(string key, string value){
	
	string  retVal = value;

	string suffix =  unitSuffixForKey(key);

	switch(unitsForKey(key)){

		case DEGREES_C:
		{
			double val = normalizedDoubleForValue(key, value);
			double tempF =  val * 9.0 / 5.0 + 32.0;
			
			char buffer[12];
			sprintf(buffer, "%3.2f%s", tempF, "°F");
			retVal = string(buffer);
		}
			break;

		case SECONDS:
		case MINUTES:
		{
			int val  = intForValue(key, value);
			if(val == -1){
				retVal = "Infinite";
			}
			else
			{
				char buffer[64];
				sprintf(buffer, "%d %s", val, suffix.c_str());
				retVal = string(buffer);
			}
		}
			break;
			
		default:
		break;
	}
	
	return retVal;
}


void CoopMgrDB::dumpMap(){
	
	timestamp::TimeStamp ts;
	
	printf("\n -- %s --\n", ts.logFileString().c_str());
	
	for (auto& [key, value] : _values) {
		
		auto lastpair = _values[key];
		auto count = _values.count(key);
		
		string desc = "";
		if(_schema.count(key)){
			desc = _schema[key].description;
		}
		
		printf("%3d %-8s:%10s %s\n",
				 (int)count,
				 key.c_str(),
				 lastpair.second.c_str(),
				 desc.c_str()
				 );
	}
}
// MARK: - Historical Events
bool CoopMgrDB::logHistoricalEvent(h_event_t evt, time_t when ){
	
	if(when == 0)
		when = time(NULL);

	auto ts = TimeStamp(when);

//printf("%s \t EVT: %s\n", ts.ISO8601String().c_str(), displayStringForHistoricalEvent(evt).c_str());
			  
	string sql = string("INSERT INTO EVENT_LOG (EVENT,DATE) ")
			+ "VALUES  (" + to_string(evt) + ", '" + ts.ISO8601String() + "' );";
	
//  printf("%s\n", sql.c_str());
	
	char *zErrMsg = 0;
	if(sqlite3_exec(_sdb,sql.c_str(),NULL, 0, &zErrMsg  ) != SQLITE_OK){
		LOGT_ERROR("sqlite3_exec FAILED: %s\n\t%s", sql.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_free(zErrMsg);
		return false;
	}
	
	return true;
}

string CoopMgrDB::displayStringForHistoricalEvent(h_event_t evt){
	
	string result = "UNKNOWN";
	switch (evt) {

		case EV_START:
			result = "START";
			break;

		case EV_SHUTDOWN:
			result = "SHUTDOWN";
			break;

		default:;
	
	}
	
	return result;
}

// MARK: -  SOLAR EVENTS

bool CoopMgrDB::refreshSolarEvents(){
	
	string str;
	bool success = false;
	
	success = getProperty(string(CoopMgrDB::PROP_LATLONG), &str);
	if(success && !str.empty()){

		double latitude, longitude;
		int n;

		if( sscanf(str.c_str(), "%lf %lf%n", &latitude, &longitude, &n) == 2) {
			SolarTimeMgr::shared()->setLatLong(latitude ,longitude);
			return SolarTimeMgr::shared()->calculateSolarEvents();
		}
	}
		return false;
}


// MARK: -  DATABASE OPERATIONS

bool CoopMgrDB::restoreValuesFromDB(){
	
	std::lock_guard<std::mutex> lock(_mutex);
	bool	statusOk = true;;
 
	_values.clear();
 
	string sql = string("SELECT NAME, VALUE, MAX(strftime('%s', DATE)) AS DATE FROM SENSOR_DATA GROUP BY NAME;");
	
	sqlite3_stmt* stmt = NULL;
	sqlite3_prepare_v2(_sdb, sql.c_str(), -1,  &stmt, NULL);

	while ( (sqlite3_step(stmt)) == SQLITE_ROW) {
	 
		string  key = string( (char*) sqlite3_column_text(stmt, 0));
		string  value = string((char*) sqlite3_column_text(stmt, 1));
		time_t  when =  sqlite3_column_int64(stmt, 2);
		
// 		printf("%8s  %8s %ld\n",  key.c_str(),  value.c_str(), when );
		_values[key] = make_pair(when, value) ;
		_etagMap[key] = _eTag++;
	}
	sqlite3_finalize(stmt);
	
		
	return statusOk;
}
 


bool CoopMgrDB::insertValueToDB(string key, string value, time_t time ){
	
	std::lock_guard<std::mutex> lock(_mutex);
	auto ts = TimeStamp(time);
	
	string sql = string("INSERT INTO SENSOR_DATA (NAME,DATE,VALUE) ")
			+ "VALUES  ('" + key + "', '" + ts.ISO8601String() + "', '" + value + "' );";
	
// printf("%s\n", sql.c_str());
	
	char *zErrMsg = 0;
	if(sqlite3_exec(_sdb,sql.c_str(),NULL, 0, &zErrMsg  ) != SQLITE_OK){
		LOGT_ERROR("sqlite3_exec FAILED: %s\n\t%s", sql.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_free(zErrMsg);
		return false;
	}
 
	return true;
}


bool CoopMgrDB::saveUniqueValueToDB(string key, string value, time_t time ){
	
	std::lock_guard<std::mutex> lock(_mutex);
	auto ts = TimeStamp(time);
	
	string sql =
	string("BEGIN;")
	+ string("DELETE FROM SENSOR_DATA WHERE NAME = '") + key + "';"
	+ string("INSERT INTO SENSOR_DATA (NAME,DATE,VALUE) ")
			+ "VALUES  ('" + key + "', '" + ts.ISO8601String() + "', '" + value + "' );"
	+ string("COMMIT;");

	/*
	 BEGIN;
	 DELETE FROM SENSOR_DATA  WHERE NAME = 'TANK_RAW';
	 INSERT INTO SENSOR_DATA (NAME,DATE,VALUE) VALUES  ('TANK_RAW', '2021-09-19 00:52:06 GMT', '1' );
	 COMMIT;

	 */
 //	printf("%s\n", sql.c_str());
	
	char *zErrMsg = 0;
	if(sqlite3_exec(_sdb,sql.c_str(),NULL, 0, &zErrMsg  ) != SQLITE_OK){
		LOGT_ERROR("sqlite3_exec FAILED: %s\n\t%s", sql.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_free(zErrMsg);
		return false;
	}
 
	return true;
}


bool CoopMgrDB::removeHistoryForKey(string key, float days){
	
	std::lock_guard<std::mutex> lock(_mutex);
	bool success = false;
	
	string sql = string("DELETE FROM SENSOR_DATA ");
	
	if(key.size() > 0) {
		sql += "WHERE NAME ='" + key + "' ";
	}
	
	if(days > 0) {
		
		if(key.size() > 0) {
			sql += "AND ";
		}
		else {
			sql += "WHERE ";
		}
		
		sql += "DATE < datetime('now' , '-" + to_string(days) + " days'); ";
	}
	else {
		sql += ";";
	}
	sqlite3_stmt* stmt = NULL;
	
	if(sqlite3_prepare_v2(_sdb, sql.c_str(), -1,  &stmt, NULL)  == SQLITE_OK){
		
		if(sqlite3_step(stmt) == SQLITE_DONE) {
 
			int count =  sqlite3_changes(_sdb);
			LOGT_DEBUG("sqlite %s\n %d rows affected", sql.c_str(), count );
			success = true;
		}
		else
		{
			LOGT_ERROR("sqlite3_step FAILED: %s\n\t%s", sql.c_str(), sqlite3_errmsg(_sdb	) );
		}
		sqlite3_finalize(stmt);
		
	}
	else {
		LOGT_ERROR("sqlite3_prepare_v2 FAILED: %s\n\t%s", sql.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_errmsg(_sdb);
	}
	
	return success;
}

bool CoopMgrDB::historyForKey(string key, historicValues_t &valuesOut, float days, int limit){

	std::lock_guard<std::mutex> lock(_mutex);
	bool success = false;
	
	historicValues_t values;
	values.clear();

	string sql = string("SELECT strftime('%s', DATE) AS DATE, VALUE FROM SENSOR_DATA WHERE NAME = '")
	+ key + "'";
	
	if(limit){
		sql += " ORDER BY DATE DESC LIMIT " + to_string(limit) + ";";
	}
	else if(days > 0) {
		sql += " AND DATE > datetime('now' , '-" + to_string(days) + " days');";
	}
	else {
		sql += ";";
	}
	
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(_sdb, sql.c_str(), -1,  &stmt, NULL);
	
	while ( (sqlite3_step(stmt)) == SQLITE_ROW) {
		time_t  when =  sqlite3_column_int64(stmt, 0);
		string  value = string((char*) sqlite3_column_text(stmt, 1));
		values.push_back(make_pair(when, value)) ;
	}
	sqlite3_finalize(stmt);

	success = values.size() > 0;
	
	if(success){
		valuesOut = values;
	}

	
	return success;
}

bool CoopMgrDB::historyForEvents( historicEvents_t &eventsOut, float days, int limit){
	std::lock_guard<std::mutex> lock(_mutex);
	bool success = false;
	
	historicEvents_t events;
	events.clear();

	string sql = string("SELECT strftime('%s', DATE) AS DATE, EVENT FROM EVENT_LOG ");
	 
	if(limit){
		sql += " ORDER BY DATE DESC LIMIT " + to_string(limit) + ";";
	}
	else if(days > 0) {
		sql += " WHERE DATE > datetime('now' , '-" + to_string(days) + " days');";
	}
	else {
		sql += ";";
	}
	
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(_sdb, sql.c_str(), -1,  &stmt, NULL);
	
	while ( (sqlite3_step(stmt)) == SQLITE_ROW) {
		time_t  when =  sqlite3_column_int64(stmt, 0);
		int  event =  sqlite3_column_int(stmt, 1);
		events.push_back(make_pair(when, (h_event_t) event)) ;
	}
	sqlite3_finalize(stmt);

	success =  true ; //events.size() > 0;
	
	if(success){
		eventsOut = events;
	}
	
	return success;
	
}

bool CoopMgrDB::trimHistory(float days){
	
	std::lock_guard<std::mutex> lock(_mutex);
	bool success = false;
	
	string sql = string("DELETE FROM EVENT_LOG ");
		
	if(days > 0) {
		sql += "WHERE DATE < datetime('now' , '-" + to_string(days) + " days'); ";
	}
	else {
		sql += ";";
	}
	sqlite3_stmt* stmt = NULL;
	
	if(sqlite3_prepare_v2(_sdb, sql.c_str(), -1,  &stmt, NULL)  == SQLITE_OK){
		
		if(sqlite3_step(stmt) == SQLITE_DONE) {
 
			int count =  sqlite3_changes(_sdb);
			LOGT_DEBUG("sqlite %s\n %d rows affected", sql.c_str(), count );
			success = true;
		}
		else
		{
			LOGT_ERROR("sqlite3_step FAILED: %s\n\t%s", sql.c_str(), sqlite3_errmsg(_sdb	) );
		}
		sqlite3_finalize(stmt);
		
	}
	else {
		LOGT_ERROR("sqlite3_prepare_v2 FAILED: %s\n\t%s", sql.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_errmsg(_sdb);
	}
	
	return success;
}



// MARK: -  SCHEMA


bool CoopMgrDB::initLogDatabase(string filePath){
	
	// create a file path
	if(filePath.size() == 0)
		filePath = "coop.db";

	LOGT_DEBUG("OPEN database: %s", filePath.c_str());

	//  Open database
	if(sqlite3_open(filePath.c_str(), &_sdb) != SQLITE_OK){
		LOGT_ERROR("sqlite3_open FAILED: %s", filePath.c_str(), sqlite3_errmsg(_sdb	) );
		return false;
	}
	
	// make sure primary tables are there.
	string sql1 = "CREATE TABLE IF NOT EXISTS SENSOR_DATA("  \
						"NAME 			  TEXT     	NOT NULL," \
						"DATE          DATETIME	NOT NULL," \
						"VALUE         TEXT     	NOT NULL);";
	
	char *zErrMsg = 0;
	if(sqlite3_exec(_sdb,sql1.c_str(),NULL, 0, &zErrMsg  ) != SQLITE_OK){
		LOGT_ERROR("sqlite3_exec FAILED: %s\n\t%s", sql1.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_free(zErrMsg);
		return false;
	}
	// make sure primary tables are there.
	string sql2 = "CREATE TABLE IF NOT EXISTS EVENT_LOG("  \
						"EVENT 		  INTEGER    NOT NULL," \
						"DATE          DATETIME	NOT NULL);";
	
	if(sqlite3_exec(_sdb,sql2.c_str(),NULL, 0, &zErrMsg  ) != SQLITE_OK){
		LOGT_ERROR("sqlite3_exec FAILED: %s\n\t%s", sql2.c_str(), sqlite3_errmsg(_sdb	) );
		sqlite3_free(zErrMsg);
		return false;
	}

	
	if(!restoreValuesFromDB()){
		LOGT_ERROR("restoreValuesFromDB FAILED");
		return false;
	}
	
	return true;
}

bool CoopMgrDB::initSchemaFromFile(string filePath){
	bool	statusOk = false;
	
	std::ifstream	ifs;
	
	// create a file path
	if(filePath.size() == 0)
		filePath = "valueschema.csv";
 
	LOGT_DEBUG("READ valueschema: %s", filePath.c_str());
	
	try{
		string line;
		std::lock_guard<std::mutex> lock(_mutex);
	
		_schema.clear();

		// open the file
		ifs.open(filePath, ios::in);
		if(!ifs.is_open()) return false;
	
		while ( std::getline(ifs, line) ) {
			
			// split the line looking for a token: and rest and ignore comments
			line = Utils::trimStart(line);
			if(line.size() == 0) continue;
			if(line[0] == '#')  continue;
			
			vector<string> v = split<string>(line, ",");
			if(v.size() != 5)  continue;
			
			string key = v[0];
			string typ = v[1];
			string track = v[2];
			string thesh = v[3];
			string desc  = Utils::trimStart(v[4]);
			
			
			if(_schemaMap.count(typ)){
				addSchema(key, _schemaMap[typ],
							 atof(thesh.c_str()),
							 desc,
							 (valueTracking_t) std::stoi( track ));
			}
		}
		
		statusOk = _schema.size() > 0;
		ifs.close();
	}
	catch(std::ifstream::failure &err) {
		
		LOGT_ERROR("READ valueschema:FAIL: %s", err.what());
		statusOk = false;
	}
	
	return statusOk;
	
}


void CoopMgrDB::addSchema(string key,
								  valueSchemaUnits_t units,
								  double threshold,
								  string description,
								  valueTracking_t tracking){
	
	valueSchema_t sc;
	sc.units = units;
	sc.theshold = threshold;
	sc.description = description;
	sc.tracking = tracking;
	
	_schema[key] = sc;
}


// MARK: - properties
bool CoopMgrDB::setProperty(string key, string value){
	_properties[key] = value;
	savePropertiesToFile();
	
	if(key ==  CoopMgrDB::PROP_LATLONG){
		refreshSolarEvents();
	}
 
	return true;
}

bool CoopMgrDB::removeProperty(string key){
	
	if(_properties.count(key)){
		_properties.erase(key);
		savePropertiesToFile();
		
		if(key ==  CoopMgrDB::PROP_LATLONG){
			SolarTimeMgr::shared()->setLatLong(numeric_limits<double>::max() ,numeric_limits<double>::max());
 		}
	 
		return true;
	}
	return false;
}

bool CoopMgrDB::setPropertyIfNone(string key, string value){
	
	if(_properties.count(key) == 0){
		_properties[key] = value;
		savePropertiesToFile();
		return true;
	}
	return false;
}

map<string ,string> CoopMgrDB::getProperties(){
	
	return _properties;
}

bool CoopMgrDB::getProperty(string key, string *value){
	
	if(_properties.count(key)){
		if(value)
			*value = _properties[key];
		return true;
	}
	return false;
}

bool  CoopMgrDB::getUint16Property(string key, uint16_t * valOut){
	
	string str;
	if(getProperty(string(key), &str)){
		char* p;
		long val = strtoul(str.c_str(), &p, 0);
		if(*p == 0 && val <= UINT16_MAX){
			if(valOut)
				*valOut = (uint16_t) val;
			return true;
		}
	}
	return false;
}

bool  CoopMgrDB::getFloatProperty(string key, float * valOut){
	
	string str;
	if(getProperty(string(key), &str)){
		char* p;
		float val = strtof(str.c_str(), &p);
		if(*p == 0){
			if(valOut)
				*valOut = (float) val;
			return true;
		}
	}
	return false;
}
 
bool  CoopMgrDB::getBoolProperty(string key, bool * valOut){
	
	string str;
	if(getProperty(string(key), &str) ){
		char* p;
		
		transform(str.begin(), str.end(), str.begin(), ::tolower);
		
		long val = strtoul(str.c_str(), &p, 0);
		if(*p == 0 && (val == 0 || val == 1)){
			if(valOut) *valOut = (bool)val;
			return true;
			
		}else if(str == "true"){
			if(valOut) *valOut = true;
			return true;
		}
		else if(str == "false"){
			if(valOut)	*valOut = false;
			return true;
		}
	}
	return false;
}


//MARK: - Database Persistent opeeration
bool CoopMgrDB::restoreEventGroupFromJSON(json j){
	bool  statusOk = false;
	
	if(j.is_object()){
		if( j.contains(string(PROP_ARG_EVENT_GROUPID))
			&& j.at(string(PROP_ARG_EVENT_GROUPID)).is_string()){
			string egID = j.at(string(PROP_ARG_EVENT_GROUPID));
			eventGroupID_t eventGroupID;
			
			if( str_to_EventGroupID(egID.c_str(), &eventGroupID )
				&& !eventGroupIsValid(eventGroupID)){
				
				eventGroupInfo_t info;
				info.eventIDs.clear();
				//						_eventsGroups[eventGroupID] = info;
				
				if( j.contains(string(JSON_ARG_NAME))
					&& j.at(string(JSON_ARG_NAME)).is_string()){
					info.name = j.at(string(JSON_ARG_NAME));
				}
				
				if( j.contains(string(PROP_ARG_EVENTIDS))
					&& j.at(string(PROP_ARG_EVENTIDS)).is_array()){
					auto eventIDs = j.at(string(PROP_ARG_EVENTIDS));
					for(string str : eventIDs){
						eventID_t eventID;
						if( str_to_EventID(str.c_str(), &eventID )){
							info.eventIDs.insert(eventID);
						}
					}
				}
				
				_eventsGroups[eventGroupID] = info;
				statusOk = true;
			}
		}
	}
	return statusOk;
};


bool 	CoopMgrDB::saveEventGroupToJSON(eventGroupID_t eventGroupID, json &j ){
	
	bool  statusOk = false;
	
	if(eventGroupIsValid(eventGroupID)){
		eventGroupInfo_t* eg =  &_eventsGroups[eventGroupID];
		
		j[string(PROP_ARG_EVENT_GROUPID)] = to_hex<unsigned short>(eventGroupID);
		if(!eg->name.empty()) j[string(JSON_ARG_NAME)] =  eg->name;
		vector<string> eventIDs;
		eventIDs.clear();
		for(auto e : eg->eventIDs) eventIDs.push_back( EventGroupID_to_string(e));
		j[string(PROP_ARG_EVENTIDS)]  = eventIDs;
		statusOk = true;
  	}
	
	return statusOk;
}

bool CoopMgrDB::restorePropertiesFromFile(string filePath){

	std::ifstream	ifs;
	bool 				statusOk = false;

	if(filePath.empty())
		filePath = _propertyFilePath;

	if(filePath.empty())
		filePath = defaultPropertyFilePath();
	
	try{
		string line;
		std::lock_guard<std::mutex> lock(_mutex);
	
		// open the file
		ifs.open(filePath, ios::in);
		if(!ifs.is_open()) return false;
	
		json jP;
		ifs >> jP;
		
		for (json::iterator it = jP.begin(); it != jP.end(); ++it) {
			
			if( it.key() == PROP_EVENT && it.value().is_array()){
				for (auto& el : it.value()) {
					if(el.is_object()){
						Event evt = Event(el);
						if(evt.isValid()&& ! eventsIsValid(evt._rawEventID)){
							_events[evt._rawEventID] = evt;
						}
					}
				}
			}
			if( it.key() == PROP_EVENT_GROUPS && it.value().is_array()){
				for (auto& el : it.value()) {
					restoreEventGroupFromJSON(el);
				}
			}
			else if( it.value().is_string()){
				_properties[it.key() ] = string(it.value());
			}
		}
 		refreshSolarEvents();
		statusOk = true;
		ifs.close();
		
		// if we were sucessful, then save the filPath
		_propertyFilePath	= filePath;
	}
	catch(std::ifstream::failure &err) {
		
		LOG_INFO("restorePropertiesFromFile:FAIL: %s", err.what());
		statusOk = false;
	}
	
	return statusOk;
}

 
bool CoopMgrDB::savePropertiesToFile(string filePath){
 
	std::lock_guard<std::mutex> lock(_mutex);
	bool statusOk = false;
	
	std::ofstream ofs;
	
	if(filePath.empty())
		filePath = _propertyFilePath;

	if(filePath.empty())
		filePath = defaultPropertyFilePath();

	try{
		ofs.open(filePath, std::ios_base::trunc);
		
		if(ofs.fail())
			return false;

		json jP;

		for (auto& [key, value] : _properties) {
			jP[key] =  string(value);
		}

		if(_events.size() > 0){
			json j;
			for (const auto& [eventID, _] : _events) {
				Event* evt =  &_events[eventID];
				j.push_back(evt->JSON());
			}
			jP[string(PROP_EVENT)] = j;
 		}
		

		if(_eventsGroups.size() > 0){
			json j;
			for (const auto& [eventGroupID, _] : _eventsGroups) {
				json jEG;
				
				if(saveEventGroupToJSON(eventGroupID, jEG)){
					j.push_back(jEG);
 				}
			}
			jP[string(PROP_EVENT_GROUPS)] = j;
		}

		string jsonStr = jP.dump(4);
		ofs << jsonStr << "\n";
		
		ofs.flush();
		ofs.close();
			
		statusOk = true;
	}
	catch(std::ofstream::failure &writeErr) {
			statusOk = false;
	}

		
	return statusOk;
}

string CoopMgrDB::defaultPropertyFilePath(){
	return "coopmgr.props.json";
}


//MARK: - Events API


bool CoopMgrDB::eventsIsValid(eventID_t eid){
	
	return(_events.count(eid) > 0);
}


bool CoopMgrDB::eventSave(Event event, eventID_t* eventIDOut){
	
	std::uniform_int_distribution<long> distribution(SHRT_MIN,SHRT_MAX);
	eventID_t eid;

	do {
		eid = distribution(_rng);
	}while( _events.count(eid) > 0);

	event._rawEventID = eid;
	_events[eid] = event;
	
	savePropertiesToFile();
 
	if(eventIDOut)
		*eventIDOut = eid;

	return true;
}

 
bool CoopMgrDB::eventDelete(eventID_t eventID){
	
	if(_events.count(eventID) == 0)
		return false;
	 
//	// remove from any event groups first
//	for (const auto& [eventGroupID, _] : _eventsGroups) {
//		eventGroupInfo_t* info  =  &_eventsGroups[eventGroupID];
//
//		if(info->eventIDs.count(eventID)){
//			info->eventIDs.erase(eventID);
//		}
//	}
	
	_events.erase(eventID);
	savePropertiesToFile();
	return true;
}

bool CoopMgrDB::eventUpdate(eventID_t eventID, Event newEvent){
	
	if(_events.count(eventID) == 0)
		return false;
	
	Event* evt =  &_events[eventID];
	
	if(!newEvent._name.empty())
		evt->_name = newEvent._name;

	if( newEvent._trigger.isValid())
		evt->_trigger = newEvent._trigger;

	if( newEvent._action.isValid())
		evt->_action = newEvent._action;

	savePropertiesToFile();
	return true;
}

bool CoopMgrDB::eventSetName(eventID_t eventID, string name){
	
	if(_events.count(eventID) == 0)
		return false;
	
	Event* evt =  &_events[eventID];
	evt->_name = name;
 
	savePropertiesToFile();
	
	return true;
}

string CoopMgrDB::eventGetName(eventID_t eventID) {
	
	if(_events.count(eventID) == 0)
		return "";

	Event* evt =  &_events[eventID];
	return evt->_name;
}

bool CoopMgrDB::eventFind(string name, eventID_t* eventIDOut){
	
	for(auto e : _events) {
		auto event = &e.second;
		
		if (strcasecmp(name.c_str(), event->_name.c_str()) == 0){
				if(eventIDOut){
				*eventIDOut = e.first;
			}
			return true;
		}
	}
	return false;
}

optional<reference_wrapper<Event>> CoopMgrDB::eventsGetEvent(eventID_t eventID){

	if(_events.count(eventID) >0 ){
		return  ref(_events[eventID]);
	}
	 
	return optional<reference_wrapper<Event>> ();
}

vector<eventID_t> CoopMgrDB::allEventsIDs(){
	vector<eventID_t> events;
 
 for (const auto& [key, _] : _events) {
	 events.push_back( key);
	}
	
	return events;
}

//vector<eventID_t> CoopMgrDB::matchingEventIDs(EventTrigger trig){
//	vector<eventID_t> events;
//	
//	for (auto& [key, evt] : _events) {
//		if(evt._trigger.shouldTriggerFromDeviceEvent(trig))
//			events.push_back( key);
//	};
//		
//	return events;
//}

vector<eventID_t> CoopMgrDB::eventsMatchingAppEvent(EventTrigger::app_event_t appEvent){
	vector<eventID_t> events;

	for (auto& [key, evt] : _events) {
		if(evt._trigger.shouldTriggerFromAppEvent(appEvent))
			events.push_back( key);
	};
		
	return events;
}


vector<eventID_t> CoopMgrDB::eventsThatNeedToRun(solarTimes_t &solar, time_t localNow){
	vector<eventID_t> events;

	for (auto& [key, evt] : _events) {
		if(evt._trigger.shouldTriggerFromTimeEvent(solar, localNow))
			events.push_back( key);
	};
		
	return events;
}

vector<eventID_t> CoopMgrDB::eventsInTheFuture(solarTimes_t &solar, time_t localNow){
	vector<eventID_t> events;

	for (auto& [key, evt] : _events) {
		if(evt._trigger.shouldTriggerInFuture(solar, localNow))
			events.push_back( key);
	};
		
	return events;
}

bool CoopMgrDB::eventSetLastRunTime(eventID_t eventID, time_t localNow){
	
	if(_events.count(eventID) == 0)
		return false;
	
	Event* evt =  &_events[eventID];
	return evt->_trigger.setLastRun(localNow);
}

void CoopMgrDB::resetAllEventsLastRun() {
	
	for (auto& [_, evt] : _events) {
		evt._trigger.setLastRun(0);
	}
};
		
 
//MARK: - event group API
 
bool str_to_EventGroupID(const char* str, eventGroupID_t *eventGroupIDOut){
	bool status = false;
	
	eventID_t val = 0;
	status = sscanf(str, "%hx", &val) == 1;
	
	if(eventGroupIDOut)  {
		*eventGroupIDOut = val;
	}
	
	return status;
};
 
string  EventGroupID_to_string(eventGroupID_t eventGroupID){
		return to_hex<unsigned short>(eventGroupID);
}

// event groups
bool CoopMgrDB::eventGroupIsValid(eventGroupID_t eventGroupID){
	return (_eventsGroups.count(eventGroupID) > 0);
}

bool CoopMgrDB::eventGroupCreate(eventGroupID_t* eventGroupIDOut, const string name){
	
	std::uniform_int_distribution<long> distribution(LONG_MIN,LONG_MAX);
	eventGroupID_t eventGroupID;

	do {
		eventGroupID = distribution(_rng);
	}while( _eventsGroups.count(eventGroupID) > 0);

	eventGroupInfo_t info;
	info.name = name;
	info.eventIDs.clear();
	_eventsGroups[eventGroupID] = info;
 
	savePropertiesToFile();
	
	if(eventGroupIDOut)
		*eventGroupIDOut = eventGroupID;
	return true;
}

bool CoopMgrDB::eventGroupDelete(eventGroupID_t eventGroupID){
 
	if(_eventsGroups.count(eventGroupID) == 0)
		return false;

	_eventsGroups.erase(eventGroupID);
	savePropertiesToFile();

	return true;
}


bool CoopMgrDB::eventGroupFind(string name, eventGroupID_t* eventGroupIDOut){
	
	for(auto g : _eventsGroups) {
		auto info = &g.second;
		
		if (strcasecmp(name.c_str(), info->name.c_str()) == 0){
			if(eventGroupIDOut){
				*eventGroupIDOut =  g.first;
			}
			return true;
		}
	}
	return false;
}
 
bool CoopMgrDB::eventGroupSetName(eventGroupID_t eventGroupID, string name){

	if(_eventsGroups.count(eventGroupID) == 0)
		return false;
 
	eventGroupInfo_t* info  =  &_eventsGroups[eventGroupID];
	info->name = name;

	savePropertiesToFile();
	return true;
}


string CoopMgrDB::eventGroupGetName(eventGroupID_t eventGroupID){

	if(_eventsGroups.count(eventGroupID) == 0)
		return "";
 
	eventGroupInfo_t* info  =  &_eventsGroups[eventGroupID];
	return info->name;
}



bool CoopMgrDB::eventGroupAddEvent(eventGroupID_t eventGroupID,  eventID_t eventID){

	if(_eventsGroups.count(eventGroupID) == 0)
		return false;

	if(!eventsIsValid(eventID))
		return false;

	eventGroupInfo_t* info  =  &_eventsGroups[eventGroupID];
	info->eventIDs.insert(eventID);
	
	savePropertiesToFile();

	return true;
}

bool CoopMgrDB::eventGroupRemoveEvent(eventGroupID_t eventGroupID, eventID_t eventID){
	
	if(_eventsGroups.count(eventGroupID) == 0)
		return false;
	
	eventGroupInfo_t* info  =  &_eventsGroups[eventGroupID];
	
	if(info->eventIDs.count(eventID) == 0)
		return false;

	info->eventIDs.erase(eventID);

	savePropertiesToFile();
	return true;
}

bool CoopMgrDB::eventGroupContainsEventID(eventGroupID_t eventGroupID, eventID_t eventID){
	
	if(_eventsGroups.count(eventGroupID) == 0)
		return false;
	
	eventGroupInfo_t* info  =  &_eventsGroups[eventGroupID];
	
	return(info->eventIDs.count(eventID) != 0);
}
 
vector<eventID_t> CoopMgrDB::eventGroupGetEventIDs(eventGroupID_t eventGroupID){
	vector<eventID_t> eventIDs;

	if(_eventsGroups.count(eventGroupID) != 0){
		eventGroupInfo_t* info  =  &_eventsGroups[eventGroupID];
		std::copy(info->eventIDs.begin(), info->eventIDs.end(), std::back_inserter(eventIDs));
	}

	return eventIDs;
}

vector<eventGroupID_t> CoopMgrDB::allEventGroupIDs(){
	vector<eventGroupID_t> eventGroupIDs;
 
 for (const auto& [key, _] : _eventsGroups) {
	 eventGroupIDs.push_back( key);
	}
	
	return eventGroupIDs;
}


void CoopMgrDB::reconcileEventGroups(const solarTimes_t &solar, time_t localNow){

	// event groups prevent us from running needless events when we reboot.
	// we only run the last elligable one for setting a steady state
	
	long nowMins = (localNow - solar.previousMidnight) / SECS_PER_MIN;

	for (const auto& [eventGroupID, _] : _eventsGroups) {
		eventGroupInfo_t* info  =  &_eventsGroups[eventGroupID];

		// create a map all all events that need to run
		map <int16_t, eventID_t> eventMap;
		
		for(auto eventID : info->eventIDs ){
			Event* evt =  &_events[eventID];
			int16_t minsFromMidnight = 0;
			
			if(evt->_trigger.calculateTriggerTime(solar,minsFromMidnight)){
				if(minsFromMidnight <= nowMins){
					eventMap[minsFromMidnight] = eventID;
				}
			}
		};
	 
		if(eventMap.size() > 0){
			// delete the last one
			eventMap.erase(prev(eventMap.end()));
			
		// mark the rest as ran
			for(auto item : eventMap ){
				auto eventID  = item.second;
				eventSetLastRunTime(eventID, localNow);
			}

		}
	}
	
}
 

// MARK: -  API Secrets
bool CoopMgrDB::apiSecretCreate(string APIkey, string APISecret){
	
	return apiSecretSetSecret(APIkey,APISecret);
	 
}

bool CoopMgrDB::apiSecretSetSecret(string APIkey, string APISecret){
	
	if(!APIkey.empty() && !APISecret.empty()){
		setProperty(string(PROP_API_KEY), APIkey);
		setProperty(string(PROP_API_SECRET), APISecret);
		return true;
	}
	 
	return false;
}

bool CoopMgrDB::apiSecretDelete(string APIkey){
	
	if(!APIkey.empty()){
		removeProperty(string(PROP_API_KEY));
		removeProperty(string(PROP_API_SECRET));
		return true;
	}
	 
	return false;
}

bool CoopMgrDB::apiSecretGetSecret(string APIkey, string &APISecret){
	
	string key, secret;
	
	getProperty(string(PROP_API_KEY), &key);
	getProperty(string(PROP_API_SECRET), &secret);
 
	if(!key.empty() && !secret.empty()){
		
// we oinly have one in this DB
		if(key != APIkey)
			return false;

		APISecret = secret;
		return true;
	}
	
	return false;
}

bool CoopMgrDB::apiSecretMustAuthenticate(){
	return getProperty(string(PROP_API_KEY), NULL) &&  getProperty(string(PROP_API_SECRET),NULL);
 }


// MARK: -   JSON REQUESTS


json CoopMgrDB::schemaJSON(){
	
	json schemaList;
 
	for (auto& [key, sch] : _schema) {
 
		json entry;
		entry[string(JSON_ARG_NAME)] =   sch.description;
		entry[string(JSON_ARG_UNITS)] =   sch.units;
		entry[string(JSON_ARG_TRACKING)] =   sch.tracking;
		entry[string(JSON_ARG_SUFFIX)] =   unitSuffixForKey(key);
		schemaList[key] = entry;
	}
	
	return schemaList;
}

json CoopMgrDB::currentValuesJSON(eTag_t  eTag){
	json j;

	for (auto& [key, value] : _values) {
		
		if(eTag != 0){
			auto k = key;
			vector<string> v =  keysChangedSinceEtag(eTag);
			
			bool found = std::any_of(v.begin(), v.end(),
											 [k](std::string const& s) {return s==k;});
			if(!found) continue;
		}
	
		auto lastpair = _values[key];
	
		time_t t = lastpair.first;
	 
		json entry;
		entry[string(JSON_ARG_VALUE)] 		=   jsonForValue(key, lastpair.second);
		entry[string(JSON_ARG_DISPLAYSTR)] =   displayStringForValue(key, lastpair.second);
		entry[string(JSON_ARG_TIME)] 		=   t;
		j[key] = entry;
	}
	
	if(j.empty()){
		j = json::object();
	}
	
	return j;

 }


json CoopMgrDB::currentJSONForKey(string key){
	json j;

	if(_values.count(key)){
		auto lastpair = _values[key];

		time_t t = lastpair.first;
	 
		json entry;
		entry[string(JSON_ARG_VALUE)] 		=   jsonForValue(key, lastpair.second);
		entry[string(JSON_ARG_DISPLAYSTR)] =   displayStringForValue(key, lastpair.second);
		entry[string(JSON_ARG_TIME)] 		=   t;
		j[key] = entry;
	}

	return j;
}

	
json CoopMgrDB::jsonForValue(string key, string value){
	json j;

//	string suffix =  unitSuffixForKey(key);

	switch(unitsForKey(key)){

		case DEGREES_C:
		{
			j = value;
//			double val = normalizedDoubleForValue(key, value);
//			j = to_string(val);
		}
			break;

		case SECONDS:
		case MINUTES:
		{
			int val  = intForValue(key, value);
			j = to_string(val);
		}
			break;
			

		default:
			j = value;
		break;
	}
	return j;
}

