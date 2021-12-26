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
constexpr string_view NOUN_HISTORY 	 			= "history";
constexpr string_view NOUN_LOG	 				= "log";
constexpr string_view NOUN_EVENTS	 	 		= "events";

constexpr string_view SUBPATH_FILE	 			= "file";
constexpr string_view SUBPATH_STATE	 		= "state";


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

constexpr string_view JSON_ARG_PROPERTIES	= "properties";

constexpr string_view JSON_ARG_SCHEMA			= "schema";
constexpr string_view JSON_ARG_VALUES			= "values";
constexpr string_view JSON_ARG_EVENTS			= "events";

constexpr string_view JSON_ARG_DAYS			= "days";
constexpr string_view JSON_ARG_LIMIT			= "limit";


constexpr string_view JSON_ARG_NAME			= "name";
constexpr string_view JSON_ARG_TRACKING		= "tracking";
constexpr string_view JSON_ARG_UNITS			= "units";
constexpr string_view JSON_ARG_ETAG			= "ETag";
constexpr string_view JSON_ARG_SUFFIX			= "suffix";
constexpr string_view JSON_ARG_DISPLAYSTR	= "display";

constexpr string_view JSON_VAL_START			= "start";
constexpr string_view JSON_VAL_STOP				= "stop";

constexpr string_view JSON_VAL_STATE			= "state";  // Open/


constexpr string_view JSON_ARG_OS_SYSNAME	= "os.sysname";
constexpr string_view JSON_ARG_OS_NODENAME	= "os.nodename";
constexpr string_view JSON_ARG_OS_RELEASE	= "os.release";
constexpr string_view JSON_ARG_OS_MACHINE	= "os.machine";
constexpr string_view JSON_ARG_OS_VERSION	= "os.version";

void registerCommandsLineFunctions();
void registerServerNouns();
#endif /* ServerCommands_h */