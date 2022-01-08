//
//  Event.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/29/21.
//

#include "Event.hpp"
#include "TimeStamp.hpp"

using namespace nlohmann;

constexpr string_view JSON_ARG_ACTION			= "action";
constexpr string_view JSON_ARG_TRIGGER			= "trigger";
constexpr string_view JSON_ARG_EVENTID 		= "eventID";
constexpr string_view JSON_ARG_NAME				= "name";

constexpr string_view JSON_TIME_BASE			= "timeBase";
constexpr string_view JSON_TIME_OFFSET		 	= "mins";

constexpr string_view JSON_ARG_EVENT			= "event";
constexpr string_view JSON_EVENT_STARTUP		= "startup";
constexpr string_view JSON_EVENT_SHUTDOWN		= "shutdown";


// MARK: - EventTrigger()


EventTrigger::EventTrigger(app_event_t appEvent){
	_eventType  		= EVENT_TYPE_APP;
	_appEvent		= appEvent;
}

EventTrigger::EventTrigger(){
	_eventType  					= EVENT_TYPE_UNKNOWN;
}

EventTrigger::EventTrigger(tod_offset_t timeBase, int16_t timeOfDay){
	_eventType  					= EVENT_TYPE_TIME;
	_timeEvent.timeBase		= timeBase;
	_timeEvent.timeOfDay		= timeOfDay;
	_timeEvent.lastRun			= 0;
}

void EventTrigger::copy(const EventTrigger &evt1, EventTrigger *evt2){
		
	evt2->_eventType	 	= evt1._eventType;
	
	switch (evt1._eventType) {
		
		case EVENT_TYPE_TIME:
			evt2->_timeEvent = evt1._timeEvent;
				break;

		case EVENT_TYPE_APP:
			evt2->_appEvent = evt1._appEvent;
				break;

		default:
			break;
	}
}

EventTrigger::EventTrigger(nlohmann::json j){
	initWithJSON(j);
}

EventTrigger::EventTrigger(std::string str){
	_eventType = EVENT_TYPE_UNKNOWN;
 
	json j;
	j  = json::parse(str);
	initWithJSON(j);
}


void EventTrigger::initWithJSON(nlohmann::json j){
	
	_eventType = EVENT_TYPE_UNKNOWN;
	
	 if( j.contains(string(JSON_TIME_OFFSET))
			  && j.contains(string(JSON_TIME_BASE))) {
		
		_eventType = EVENT_TYPE_TIME;
	}
	else if(j.contains(string(JSON_ARG_EVENT))){
		_eventType = EVENT_TYPE_APP;
		_appEvent = APP_EVENT_INVALID;
	}
	
	
	if(_eventType == EVENT_TYPE_TIME) {
		
		_timeEvent.timeBase = TOD_INVALID;
		_timeEvent.timeOfDay = 0;
		_timeEvent.lastRun = 0;
		
		if( j.contains(string(JSON_TIME_BASE))
			&& j.at(string(JSON_TIME_BASE)).is_number()){
			_timeEvent.timeBase = j.at(string(JSON_TIME_BASE));
		}
		
		if( j.contains(string(JSON_TIME_OFFSET))
			&& j.at(string(JSON_TIME_OFFSET)).is_number()){
			_timeEvent.timeOfDay = j.at(string(JSON_TIME_OFFSET));
		}
	}
	
	else if(_eventType == EVENT_TYPE_APP) {
		
		if( j.contains(string(JSON_ARG_EVENT))
			&& j.at(string(JSON_ARG_EVENT)).is_string()){
			string str = j.at(string(JSON_ARG_EVENT));
			
			if(str == JSON_EVENT_STARTUP ){
				_appEvent = APP_EVENT_STARTUP;
			}
			else if(str == JSON_EVENT_SHUTDOWN ){
				_appEvent = APP_EVENT_SHUTDOWN;
			}
 		}
	}
}

nlohmann::json EventTrigger::JSON(){
	json j;
	
	switch(_eventType){
			
		case EVENT_TYPE_TIME:
		{
			json j1;
			
			j1[string(JSON_TIME_OFFSET)] =  _timeEvent.timeOfDay;
			j1[string(JSON_TIME_BASE)] 	=  _timeEvent.timeBase;
			j = j1;
		}
			break;
			
		case EVENT_TYPE_APP:
		{
			json j1;
			
			switch (_appEvent) {
				case APP_EVENT_STARTUP:
					j1[string(JSON_ARG_EVENT)] =  JSON_EVENT_STARTUP;
					break;
					
				case APP_EVENT_SHUTDOWN:
					j1[string(JSON_ARG_EVENT)] =  JSON_EVENT_SHUTDOWN;
					break;
	 
					
				default:
					break;
			}
			j = j1;
		}
			break;

		default:;
	}
	
	return j;
}

const std::string EventTrigger:: printString(){
	std::ostringstream oss;
	using namespace timestamp;

	auto j = JSON();
	
	 if(_eventType == EVENT_TYPE_TIME){

		 solarTimes_t solar;
		 SolarTimeMgr::shared()->getSolarEvents(solar);
			int16_t minsFromMidnight = 0;
		 
		 // when does it need to run today
		 calculateTriggerTime(solar,minsFromMidnight);
		time_t schedTime = solar.previousMidnight + (minsFromMidnight * SECS_PER_MIN) ;
		 string timeString = TimeStamp(schedTime).ClockString();
		 
		 string offsetMinutes = "";
		 
		 if(_timeEvent.timeOfDay > 0)
			 offsetMinutes = " + " + to_string(_timeEvent.timeOfDay) + " minutes";
		 else if(_timeEvent.timeOfDay < 0)
			 offsetMinutes =  " - " + to_string(abs(_timeEvent.timeOfDay)) + " minutes";
		
		 switch(_timeEvent.timeBase){
			 case TOD_SUNRISE:
				 oss << timeString << " (Sunrise"  << offsetMinutes << ")" ;
				 break;
				 
			 case TOD_SUNSET:
				 oss << timeString << " (Sunset"  << offsetMinutes << ")" ;
			 break;
				 
			 case TOD_CIVIL_SUNRISE:
				 oss << timeString << " (Civil Sunrise" << offsetMinutes << ")" ; ;
				 break;
				 
			 case TOD_CIVIL_SUNSET:
				 oss << timeString << " (Civil Sunset"  << offsetMinutes << ")";
				 break;
				 
			 case TOD_ABSOLUTE:
				 oss << timeString;
				 break;
				 
			 case TOD_INVALID:
				 oss <<  "Invalid Time:";
				 break;
		 }
	}
	else	{
		oss << j.dump();
	}
	return  oss.str();

}
  
bool EventTrigger::isValid(){
	return (_eventType != EVENT_TYPE_UNKNOWN);
}
bool EventTrigger::isTimed(){
	return (_eventType == EVENT_TYPE_TIME);
}

bool EventTrigger::setLastRun(time_t time){
	if(_eventType == EVENT_TYPE_TIME){
		_timeEvent.lastRun = time;
		return true;
	} else return false;
}


bool EventTrigger::shouldTriggerInFuture(const solarTimes_t &solar, time_t localNow){
	
	bool result = false;
	
	if(_eventType == EVENT_TYPE_TIME){
		
		int16_t minsFromMidnight = 0;
		
		// when does it need to run today
		if(calculateTriggerTime(solar,minsFromMidnight)) {
			time_t schedTime = solar.previousMidnight + (minsFromMidnight * SECS_PER_MIN) ;
//
//			printf("\n sched: %s \n", timestamp::TimeStamp(schedTime).ClockString().c_str());
//			printf(" now:  %s \n", timestamp::TimeStamp(localNow).ClockString().c_str());
//
			if( schedTime > localNow) {
						result = true;
			}
		};
	}
	
	return result;
	
}

bool EventTrigger::shouldTriggerFromAppEvent(app_event_t a){
	bool result = false;

	if(_eventType == EVENT_TYPE_APP){
		return (_appEvent == a);
	}
	
	return result;
}

bool EventTrigger::shouldTriggerFromTimeEvent(const solarTimes_t &solar, time_t localNow){
	
	bool result = false;
	
	if(_eventType == EVENT_TYPE_TIME){
		
		int16_t minsFromMidnight = 0;
		
		// when does it need to run today
		if(calculateTriggerTime(solar,minsFromMidnight)) {
			time_t schedTime = solar.previousMidnight + (minsFromMidnight * SECS_PER_MIN) ;
//
//			printf("\n sched: %s \n", timestamp::TimeStamp(schedTime).ClockString().c_str());
//			printf(" now:  %s \n", timestamp::TimeStamp(localNow).ClockString().c_str());
//
			if( schedTime < localNow) {
				if( _timeEvent.lastRun  == 0
					||  _timeEvent.lastRun < solar.previousMidnight )
					result = true;
			}
		};
	}
	
	return result;
}


bool EventTrigger::calculateTriggerTime(const solarTimes_t &solar, int16_t &minsFromMidnight) {
	
	bool result = false;
	
	if(_eventType == EVENT_TYPE_TIME){
		int16_t actualTime  = _timeEvent.timeOfDay;
		
		switch(_timeEvent.timeBase){
			case TOD_SUNRISE:
				actualTime = solar.sunriseMins + actualTime;
				result = true;
				break;
				
			case TOD_SUNSET:
				actualTime = solar.sunSetMins + actualTime;
				result = true;
				break;
				
			case TOD_CIVIL_SUNRISE:
				actualTime = solar.civilSunRiseMins + actualTime;
				result = true;
				break;
				
			case TOD_CIVIL_SUNSET:
				actualTime = solar.civilSunSetMins + actualTime;
				result = true;
				break;
				
			case TOD_ABSOLUTE:
				actualTime = actualTime;
				result = true;
				break;
				
			case TOD_INVALID:
				break;
		}
		
		if(result)
			minsFromMidnight = actualTime;
	}
 
	return result;
}

// MARK: - Event()
 
bool str_to_EventID(const char* str, eventID_t *eventIDOut){
	bool status = false;
	
	eventID_t val = 0;
 
	status = sscanf(str, "%hx", &val) == 1;
	
	if(eventIDOut)  {
		*eventIDOut = val;
	}
	
	return status;

};

 
string  EventID_to_string(eventID_t eventID){
		return to_hex<unsigned short>(eventID);
}


Event::Event(){
	_rawEventID = 0;
 	_trigger 	= EventTrigger();
	_action 		= Action();
}

Event::Event(EventTrigger trigger, Action action){
	_rawEventID = 0;
	_trigger = trigger;
	_action = action;
}


Event::Event(EventTrigger trigger, Action action, std::string name){
	_rawEventID = 0;
	_trigger = trigger;
	_action = action;
	if(!name.empty())
		_name = name;
}

Event::Event(nlohmann::json j){
	
	_rawEventID = 0;
	
	if( j.contains(JSON_ARG_TRIGGER)
		&& j.at(string(JSON_ARG_TRIGGER)).is_object()) {
		auto jT = j.at(string(JSON_ARG_TRIGGER));
		_trigger = EventTrigger(jT);
	}
	
	if( j.contains(JSON_ARG_ACTION)
		&& j.at(string(JSON_ARG_ACTION)).is_object()) {
		auto jA = j.at(string(JSON_ARG_ACTION));
		_action = Action(jA);
	}
	
	if(j.contains(JSON_ARG_NAME)
		&& j.at(string(JSON_ARG_NAME)).is_string()) {
		auto jS = j.at(string(JSON_ARG_NAME));
		_name = jS;
	}
	
	if(j.contains(JSON_ARG_EVENTID)
		&& j.at(string(JSON_ARG_EVENTID)).is_string()) {
		string jID = j.at(string(JSON_ARG_EVENTID));
		str_to_EventID(jID.c_str(), &_rawEventID);
	}
}


void Event::copy(const Event &evt1, Event *evt2){
		
	evt2->_rawEventID	 	= evt1._rawEventID;
	evt2->_name				= evt1._name;
 	evt2->_trigger			= evt1._trigger;
	evt2->_action			= evt1._action;
 }

 
nlohmann::json Event::JSON(){
	json j;
	
	auto jA = _action.JSON();
 	auto jT = _trigger.JSON();
	
	j[string(JSON_ARG_ACTION)] = jA;
 	j[string(JSON_ARG_TRIGGER)] = jT;
	
	j[string(JSON_ARG_EVENTID)] = to_hex<unsigned short>(_rawEventID);
	if(!_name.empty()) j[string(JSON_ARG_NAME)] = _name;
	
	return j;
}

bool Event::isValid(){
	return (_action.isValid()
			  && _trigger.isValid());
}
