//
//  Action.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/29/21.
//

#include "Action.hpp"
#include "CoopDevices.hpp"
#include "Utils.hpp"
#include <regex>

using namespace nlohmann;
using namespace std;


bool str_to_ActionID(const char* str, actionID_t *actionIDOut){
	
	bool status = false;
	
	actionID_t val = 0;
 
	status = sscanf(str, "%hx", &val) == 1;
	
	if(actionIDOut)  {
		*actionIDOut = val;
	}
	
	return status;
}

Action::Action(){
	_cmd = string();
	_deviceID = string();
}

Action::Action(string deviceID, string cmd){
	_cmd = cmd;
	_deviceID = deviceID;
}
 

Action::Action(json j) {
	initWithJSON(j);
}


Action::Action(std::string str){
	
	_cmd = string();
	_deviceID = string();
	
	json j;
	j  = json::parse(str);
	initWithJSON(j);
}



std::string Action::idString() const {
	return  to_hex<unsigned short>(_actionID);
}

void Action::initWithJSON(nlohmann::json j){
	_cmd = string();
	_deviceID = string();
 
	if( j.contains(string(JSON_CMD))
		&& j.at(string(JSON_CMD)).is_string()){
 		string str  = j.at(string(JSON_CMD));
 		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		_cmd = str;

	}
	
	if( j.contains(string(JSON_DEVICEID))
		&& j.at(string(JSON_DEVICEID)).is_string()){
		string str  = j.at(string(JSON_DEVICEID));
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		_deviceID = str;
	}
	if( j.contains(string(JSON_ACTIONID))
		&& j.at(string(JSON_ACTIONID)).is_string()){
		string str = j.at(string(JSON_ACTIONID));
		
		actionID_t actionID = 0;
		if( regex_match(string(str), std::regex("^[A-Fa-f0-9]{4}$"))
			&& ( std::sscanf(str.c_str(), "%hd", &actionID) == 1)){
			_actionID = actionID;

		}
		else if( regex_match(string(str), std::regex("^0?[xX][0-9a-fA-F]{4}$"))
				  && ( std::sscanf(str.c_str(), "%hx", &actionID) == 1)){
			_actionID = actionID;
		}
		 
	}
 }
	
const nlohmann::json Action::JSON(){
	json j;
	
	j[string(JSON_DEVICEID)] = _deviceID;
	j[string(JSON_CMD)] = _cmd;
	return j;
}

std::string Action::printString() const {
	std::ostringstream oss;

	oss << _deviceID  << "  " << _cmd;
	return  oss.str();

}
