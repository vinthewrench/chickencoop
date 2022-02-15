//
//  ServerCommands.h
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/20/21.
//

#ifndef ServerCommands_h
#define ServerCommands_h


constexpr string_view NOUN_VERSION		 		= "version";
constexpr string_view NOUN_DATE		 			= "date";
constexpr string_view NOUN_STATE		 			= "state";
constexpr string_view NOUN_DEVICES	 			= "devices";

constexpr string_view NOUN_SCHEMA	 			= "schema";
constexpr string_view NOUN_VALUES	 			= "values";
constexpr string_view NOUN_PROPERTIES	 		= "props";
constexpr string_view NOUN_VALUE_HISTORY 	 	= "valuehistory";
constexpr string_view NOUN_LOG	 				= "log";
constexpr string_view NOUN_HISTORY	 	 		= "history";
constexpr string_view NOUN_EVENTS				= "events";
constexpr string_view NOUN_EVENTS_GROUPS		= "event.groups";

constexpr string_view SUBPATH_FILE	 			= "file";
constexpr string_view SUBPATH_STATE	 			= "state";
constexpr string_view SUBPATH_DOOR 				= "door";
constexpr string_view SUBPATH_LIGHT 			= "light";
constexpr string_view SUBPATH_AUX 				= "aux";

constexpr string_view SUBPATH_POWER 			= "power";

constexpr string_view SUBPATH_RUN_ACTION		= "run.actions";

constexpr string_view JSON_ARG_FILEPATH		= "filepath";
constexpr string_view JSON_ARG_LOGFLAGS		= "logflags";
constexpr string_view JSON_ARG_MESSAGE		= "message";			// for logfile

constexpr string_view JSON_ARG_DATE			= "date";
constexpr string_view JSON_ARG_VERSION		= "version";
constexpr string_view JSON_ARG_BUILD_TIME	= "buildtime";
constexpr string_view JSON_ARG_UPTIME		= "uptime";

constexpr string_view JSON_ARG_STATE			= "state";
constexpr string_view JSON_ARG_STATESTR		= "stateString";
constexpr string_view JSON_ARG_CPU_TEMP		= "cpuTemp";

constexpr string_view JSON_ARG_COOP_DEVICE	 	= "coopdevice";
constexpr string_view JSON_ARG_TEMP_SENSOR1	 	= "tempsensor1";

constexpr string_view JSON_ARG_DOOR	 			= "door";
constexpr string_view JSON_ARG_LIGHT	 		= "light";
constexpr string_view JSON_ARG_AUX	 		= "aux";
constexpr string_view JSON_ARG_COOP_TEMP	 	= "coopTemp";

constexpr string_view JSON_ARG_PROPERTIES	= "properties";

constexpr string_view JSON_ARG_SCHEMA			= "schema";
constexpr string_view JSON_ARG_VALUES			= "values";
constexpr string_view JSON_ARG_HISTORICAL_EVENTS			= "historicalevents";

constexpr string_view JSON_ARG_DAYS			= "days";
constexpr string_view JSON_ARG_LIMIT			= "limit";
 
constexpr string_view JSON_ARG_ACTIONID 		= "actionID";
constexpr string_view JSON_ARG_EVENTID 		= "eventID";
constexpr string_view JSON_ARG_EVENTIDS		= "eventIDs";
constexpr string_view JSON_ARG_GROUPID 		= "groupID";
constexpr string_view JSON_ARG_GROUPIDS		= "groupIDs";

constexpr string_view JSON_ARG_ACTION			= "action";
constexpr string_view JSON_ARG_TRIGGER			= "trigger";
constexpr string_view JSON_ARG_TIMED_EVENTS	= "events.timed";
constexpr string_view JSON_ARG_FUTURE_EVENTS	= "events.future";
constexpr string_view JSON_ARG_ACTIONS		= "actions";


constexpr string_view JSON_ARG_NAME			= "name";
constexpr string_view JSON_ARG_TRACKING		= "tracking";
constexpr string_view JSON_ARG_UNITS			= "units";
constexpr string_view JSON_ARG_ETAG			= "ETag";
constexpr string_view JSON_ARG_SUFFIX			= "suffix";
constexpr string_view JSON_ARG_DISPLAYSTR	= "display";

constexpr string_view JSON_VAL_START			= "start";
constexpr string_view JSON_VAL_STOP				= "stop";

constexpr string_view JSON_VAL_STATE			= "state";  // Open/

constexpr string_view JSON_ARG_VIN			= "voltage.in";
constexpr string_view JSON_ARG_VOUT			= "voltage.out";
constexpr string_view JSON_ARG_IOUT			= "current.out";
constexpr string_view JSON_ARG_P_MODE		= "powermode";
constexpr string_view JSON_ARG_TEMPC		= "tempc";

constexpr string_view JSON_ARG_SOC				= "SOC";
constexpr string_view JSON_ARG_PIJUICE_STATUS = "pijuice.status";
constexpr string_view JSON_ARG_PIJUICE_FAULT = "pijuice.fault";
constexpr string_view JSON_ARG_BATTERY_TEMP = "battery.temp";
constexpr string_view JSON_ARG_BATTERY_VOLT = "battery.volt";
constexpr string_view JSON_ARG_PIJUICE_PIN1 = "pijuice.pin1";
constexpr string_view JSON_ARG_PIJUICE_PIN2 = "pijuice.pin2";

constexpr string_view JSON_ARG_OS_SYSNAME	= "os.sysname";
constexpr string_view JSON_ARG_OS_NODENAME	= "os.nodename";
constexpr string_view JSON_ARG_OS_RELEASE	= "os.release";
constexpr string_view JSON_ARG_OS_MACHINE	= "os.machine";
constexpr string_view JSON_ARG_OS_VERSION	= "os.version";

void registerCommandsLineFunctions();
void registerServerNouns();
#endif /* ServerCommands_h */
