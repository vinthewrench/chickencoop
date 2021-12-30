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
					// these shouldnt change, 		they become persistant
	typedef enum  {
		ACTION_INALID 					= 0,
		ACTION_NONE						= 1,
		ACTION_DOOR						= 2,
		ACTION_LIGHT  					= 3,
	}actionCmd_t;
 
	constexpr static const string_view JSON_ACTIONID 	= "actionID";
	constexpr static string_view JSON_CMD_LIGHT			= "light";
	constexpr static string_view JSON_CMD_DOOR			= "door";
	constexpr static string_view JSON_CMD_NONE			= "none";
	
	Action();
	Action(actionCmd_t cmd, bool relayState = false);
	Action(nlohmann::json j);
	Action(std::string);

	Action( const Action &actIn){
		_cmd = actIn._cmd;
		_relayState =  actIn._relayState;
		_actionID = actIn._actionID;
	}

	inline void operator = (const Action &right ) {
		_cmd = right._cmd;
		_relayState =  right._relayState;
		_actionID = right._actionID;
	}
	
	std::string idString() const;
	std::string printString() const;
	const nlohmann::json JSON();
	
	bool isValid() {return (_cmd != ACTION_INALID);};
 
	const actionID_t 		actionID(){return _actionID;};
	const bool 				relayState(){return _relayState;};
	const actionCmd_t 	cmd(){return _cmd;};

	
private:
	void initWithJSON(nlohmann::json j);
	
protected:
	actionCmd_t _cmd;
	bool	 		_relayState;
	actionID_t	_actionID;
	};


#endif /* Action_hpp */
