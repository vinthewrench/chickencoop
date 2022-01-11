//
//  DoorMgr.hpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/27/21.
//

#ifndef DoorMgr_hpp
#define DoorMgr_hpp

#include <stdio.h>
#include <functional>
#include <string>
#include <sys/time.h>
#include <map>
#include <vector>


//#include "CoopDevices.hpp"

#include "CoopMgrCommon.h"

using namespace std;

class CoopDevices;
class DoorMgr {
	
	
public:

	typedef enum  {
		STATE_UNKNOWN = 0,
		STATE_OPEN,
		STATE_OPENING,
		STATE_CLOSED,
		STATE_CLOSING,
				
		ANY_STATE  = 200, // used for state processing ..
		NO_NEW_STATE

	}state_t;

	DoorMgr(CoopDevices *coopDev);
	~DoorMgr();

	
	bool begin();

	void startOpen(boolCallback_t callback = NULL);
	void startClose(boolCallback_t callback = NULL);
	void stop(boolCallback_t callback = NULL);

	state_t doorState() { return _currentState;};
	
	void idle(); 	// called from loop
	void reset(); 	// reset from timeout
	

private:

	typedef enum {
		EV_NONE = 0,
		EV_INIT,			// Init system
		EV_BUTTON,			// button push / toggle
		EV_OPEN,
		EV_CLOSE,
	} event_t ;

	
	typedef enum {
		FN_INIT = 0,
		FN_NULL,
		FN_OPEN,
		FN_CLOSE,
		FN_OPENING,
		FN_CLOSING,
		FN_BUTTON,	//toggle button
	} action_t ;

	typedef struct
	{
		state_t		 		current_state;
		event_t		 	 	received_event;
		action_t 			action;
		state_t		 		next_state;
	} state_table_t;


	state_t 						_currentState;
	vector<state_table_t> 	_state_table;
	time_t						_lastDoorAction;
	
	void receive_event(event_t event, boolCallback_t callback = NULL);

	bool do_action(action_t action);
	
	bool action_init();
	bool action_button();
	bool action_open();
	bool action_close();
	bool action_closing();
	bool action_opening();
	bool action_null();

	bool doOpen(boolCallback_t callback = NULL);
	bool doClose(boolCallback_t callback = NULL);
	
	CoopDevices* _coopDev;
};

#endif /* DoorMgr_hpp */
