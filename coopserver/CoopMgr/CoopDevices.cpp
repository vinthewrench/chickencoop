//
//  CoopDevices.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/21/21.
//

#include "CoopDevices.hpp"
#include "LogMgr.hpp"

CoopDevices::CoopDevices(){
	_state = INS_UNKNOWN;
}

CoopDevices::~CoopDevices(){
	stop();
}

 
bool CoopDevices::begin(int *error){
	bool status = false;

		

	return status;
}

void CoopDevices::stop(){
 
}

bool CoopDevices::isConnected(){
	return  false;
}
 
void CoopDevices::reset(){
	
}

void CoopDevices::idle(){
	
}

CoopMgrDevice::device_state_t CoopDevices::getDeviceState(){
  
  device_state_t retval = DEVICE_STATE_UNKNOWN;
  
  if(!isConnected())
	  retval = DEVICE_STATE_DISCONNECTED;
  
  else if(_state == INS_INVALID)
	  retval = DEVICE_STATE_ERROR;
  
  else retval = DEVICE_STATE_CONNECTED;

  return retval;
}
