//
//  ErrorMgr.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 2/27/22.
//

#include "ErrorMgr.hpp"
#include "CoopMgr.hpp"
#include "CoopMgrDB.hpp"

#include "LogMgr.hpp"

ErrorMgr *ErrorMgr::sharedInstance = NULL;


ErrorMgr::ErrorMgr(){
	
}

ErrorMgr::~ErrorMgr(){
	
}

  

void ErrorMgr::logError( level_t 	level,
								facility_t 	facility,
								uint8_t 		deviceID,
								int  			errnum,
								const char *format, ...){
	
	
	char *s;
	va_list args;
	va_start(args, format);
	vasprintf(&s, format, args);

	string errorMsg;
	
	if(errnum > 0){
		errorMsg = string(strerror(errnum));
	}
	basic_string <char> str(s, strlen(s));
	
	CoopMgr::shared()->_db.logError(level, facility,  deviceID,  str, errorMsg );
	
	free(s);
	va_end(args);
}



