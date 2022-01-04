//
//  Action.hpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/29/21.
//

#ifndef Action_hpp
#define Action_hpp

#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <map>

#include "json.hpp"
 
using namespace std;

typedef  unsigned short actionID_t;

bool str_to_ActionID(const char* str, actionID_t *actionIDOut = NULL);

class Action {
	friend class CoopMgrDB;
 
public: 
	constexpr static const string_view JSON_ACTIONID 	= "actionID";
	constexpr static string_view JSON_CMD					= "cmd";
	constexpr static string_view JSON_DEVICEID 			= "deviceID";


		
	Action();
	Action(string deviceID, string cmd);
	Action(nlohmann::json j);
	Action(std::string);

	Action( const Action &actIn){
		_cmd = actIn._cmd;
		_deviceID = actIn._deviceID;
		_actionID = actIn._actionID;
	}

	inline void operator = (const Action &right ) {
		_cmd = right._cmd;
		_deviceID = right._deviceID;
		_actionID = right._actionID;
	}
	
	std::string idString() const;
	std::string printString() const;
	const nlohmann::json JSON();
	
	bool isValid() {return !(_cmd.empty() || _deviceID.empty());};
 
	const actionID_t 		actionID(){return _actionID;};
	const string			cmd() { return _cmd;};
	const string 			deviceID(){return 	_deviceID;};

	
private:
	void initWithJSON(nlohmann::json j);
	
protected:
	string 		_cmd;
	string 		_deviceID;
	actionID_t	_actionID;
	};


#endif /* Action_hpp */
