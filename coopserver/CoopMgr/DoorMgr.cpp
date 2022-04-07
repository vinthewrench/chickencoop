//
//  DoorMgr.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/27/21.
//

#include "DoorMgr.hpp"
#include "CoopDevices.hpp"
#include "LogMgr.hpp"

 
const time_t open_delay  	= 30;
const time_t close_delay  	= 30;


DoorMgr::DoorMgr( CoopDevices *coopDev){
	_coopDev = coopDev;

	_state_table = {
		{ANY_STATE , EV_INIT, FN_INIT, NO_NEW_STATE},
		{STATE_OPENING , EV_NONE, FN_OPENING, NO_NEW_STATE},
		{STATE_CLOSING , EV_NONE,FN_CLOSING, NO_NEW_STATE},
		{ANY_STATE, EV_BUTTON, FN_BUTTON, NO_NEW_STATE},
		{ANY_STATE , EV_OPEN,  FN_OPEN, NO_NEW_STATE},
		{ANY_STATE , EV_CLOSE,FN_CLOSE, NO_NEW_STATE},
		{ANY_STATE , EV_NONE, FN_NULL, NO_NEW_STATE},
	};
	
	_currentState = STATE_UNKNOWN;
	_lastDoorAction = 0;
	
}

DoorMgr::~DoorMgr(){
 }

 bool DoorMgr::begin(){
	receive_event(EV_INIT);
	 return true;
}


void DoorMgr::idle()
{
	// idle event to run the states
	receive_event(EV_NONE);
}

void DoorMgr::startOpen(boolCallback_t cb){
	receive_event(EV_OPEN, [=]( bool didSucceed) {
		if(cb) (cb)(didSucceed);
	});
}

void DoorMgr::startClose(boolCallback_t cb){
	receive_event(EV_CLOSE, [=]( bool didSucceed) {
		if(cb) (cb)(didSucceed);
	});
}

void DoorMgr::reset(){
	receive_event(EV_INIT);

}

void DoorMgr::stop(boolCallback_t cb){
	
	_currentState = STATE_UNKNOWN;
	_lastDoorAction = 0;

	 _coopDev->stopDoor([=]( bool didSucceed) {
		if(cb) (cb)(didSucceed);
	});
}


void DoorMgr::receive_event(event_t event, boolCallback_t cb ){
	
//	if(event != EV_NONE) LOGT_INFO("RCV_EVENT %d", event);

	for(auto entry : _state_table){
		
		if ( ( event == entry.received_event ) &&
			 ( ( _currentState == entry.current_state ) ||
			  ( entry.current_state == ANY_STATE ) ) )
		{
			
			if( do_action(entry.action)) {
				if(entry.next_state != NO_NEW_STATE){
					_currentState = entry.next_state;
				}
				if(cb) (cb)(true);
				return;
			}
		}
	}
	
	if(cb) (cb)(false);
	
}


bool DoorMgr::do_action(action_t action){
	switch (action) {
			
		case FN_INIT:
			return action_init();
 
		case FN_NULL:
			return action_null();
 
		case FN_OPEN:
			return action_open();

		case FN_CLOSE:
			return action_close();

		case FN_OPENING:
			return action_opening();

		case FN_CLOSING:
			return action_closing();

		case FN_BUTTON:
			return action_button();

	}
	return false;
}


bool DoorMgr::action_init(){
	
	
	
	// starting the door manager we close for safety
//	stop([=]( bool didSucceed) {
//		// close the door
//		
//		action_close();
// 
////
////		// if the door is closed keep it that way
////		if(door.isClosed()) {
////			self.currentState = STATE_CLOSED;
////		}
////		// open it.
////		else {
////			Open_Action(self);
////		}
//	
//	});
// 
	
	return true;
}

// do nothing
bool DoorMgr::action_null(){
	return true;
}

bool DoorMgr::action_button(){
	return true;
}

bool DoorMgr::action_open(){
	// close the door
	doOpen();
	_lastDoorAction = time(NULL);
	_currentState = STATE_OPENING;
	
	return true;
}

bool DoorMgr::action_close(){
	
	// close the door
	doClose();
	_lastDoorAction = time(NULL);
	_currentState = STATE_CLOSING;
	
	return true;
}

bool DoorMgr::action_closing(){
	
	if(_lastDoorAction == 0)
		return false;
	
	time_t now =  time(NULL);
	
	if(now > (_lastDoorAction + close_delay)){
		stop([=]( bool didSucceed) {
			_currentState = STATE_CLOSED;
			_lastDoorAction = 0;
		});
	}

	return true;
}

bool DoorMgr::action_opening(){
	
	if(_lastDoorAction == 0)
		return false;
	
	time_t now =  time(NULL);
	
	if(now > (_lastDoorAction + open_delay)){
		stop([=]( bool didSucceed) {
			_currentState = STATE_OPEN;
			_lastDoorAction = 0;
		});
	}
	return true;
}
 

bool DoorMgr::doOpen(boolCallback_t cb){
	
	return _coopDev->setDoor(true, [=]( bool didSucceed) {
		
		if(cb) (cb)(didSucceed);
	});
}

bool DoorMgr::doClose(boolCallback_t cb){
	
	return _coopDev->setDoor(false, [=]( bool didSucceed) {
		
		if(cb) (cb)(didSucceed);
	});

}

