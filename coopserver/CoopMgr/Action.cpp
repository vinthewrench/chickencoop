//
//  Action.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/29/21.
//

#include "Action.hpp"
#include "CoopDevices.hpp"
#include "Utils.hpp"
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

//static std::string stringForCmd(Action::actionCmd_t cmd) {
//	
//	string str = "";
//	
//	switch(cmd) {
//		case Action::ACTION_NONE:
//			str = Action::JSON_CMD_NONE;
//			break;
//		case Action::ACTION_DOOR:
//			str = Action::JSON_CMD_DOOR;
//			break;
//		case Action::ACTION_LIGHT:
//			str = Action::JSON_CMD_LIGHT;
//			break;
//
//		default:;
//	}
//	return  str;
//}

Action::Action(){
	_cmd = ACTION_INALID;
	_relayState = false;
}

Action::Action(actionCmd_t cmd, bool relayState){
 	_cmd = cmd;
	_relayState = relayState;
}


Action::Action(json j) {
	initWithJSON(j);
}


Action::Action(std::string str){
	
	_cmd = ACTION_NONE;
	_relayState = false;
	
	json j;
	j  = json::parse(str);
	initWithJSON(j);
}



std::string Action::idString() const {
	return  to_hex<unsigned short>(_actionID);
}

void Action::initWithJSON(nlohmann::json j){
	_cmd = ACTION_NONE;
	_relayState = false;
	
	if( j.contains(string(JSON_CMD_LIGHT))){
		
		bool relayState = false;
		if(CoopDevices::jsonToRelayState(j.at(string(JSON_CMD_LIGHT)), &relayState)){
			_cmd  = ACTION_LIGHT;
			_relayState = relayState;
		}
	}
	else 	if( j.contains(string(JSON_CMD_DOOR))){
		
		bool relayState = false;
		if(CoopDevices::jsonToRelayState(j.at(string(JSON_CMD_DOOR)), &relayState)){
			_cmd  = ACTION_DOOR;
			_relayState = relayState;
		}
	}
}
	
const nlohmann::json Action::JSON(){
	json j;
	
	switch (_cmd) {
		case ACTION_DOOR:
			j[string(JSON_CMD_DOOR)] = _relayState?"open":"close";
			break;
			
		case ACTION_LIGHT:
			j[string(JSON_CMD_LIGHT)] = _relayState?"on":"off";
			break;

		case ACTION_NONE:
			j[string(JSON_CMD_NONE)] = NULL;
			break;

		default: break;
	}
 
	return j;
}

std::string Action::printString() const {
	std::ostringstream oss;

	switch (_cmd) {
		case ACTION_DOOR:
			oss << JSON_CMD_DOOR  << "  " <<  (_relayState?"open":"close");
			break;
			
		case ACTION_LIGHT:
			oss << JSON_CMD_LIGHT  << "  " <<  (_relayState?"on":"off");
 			break;

		case ACTION_NONE:
			oss << JSON_CMD_NONE;
	 			break;

		default: break;
	}

	return  oss.str();

}
