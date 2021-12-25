//
//  QwiicButton.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/21/21.
//

#include "QwiicButton.hpp"


QwiicButton::QwiicButton(){
	_isSetup = false;
}

QwiicButton::~QwiicButton(){
	stop();
	
}

bool QwiicButton::begin(uint8_t deviceAddress){
	int error = 0;

	return begin(deviceAddress, error);
}

 
bool QwiicButton::begin(uint8_t deviceAddress,   int &error){
  
	_isSetup = _i2cPort.begin(deviceAddress, error);
	
// 	LOG_INFO("QwiicButton(%02x) begin: %s\n", deviceAddress, _isSetup?"OK":"FAIL");
 
  return _isSetup;
}
 
void QwiicButton::stop(){
//	LOG_INFO("QwiicButton(%02x) stop\n",  _i2cPort.getDevAddr());

	_isSetup = false;
	_i2cPort.stop();
}
 
bool QwiicButton::isOpen(){
	return _isSetup;
	
};


uint8_t	QwiicButton::getDevAddr(){
	return _i2cPort.getDevAddr();
};


bool QwiicButton::isPressed()
{
	return false;
}


 
bool QwiicButton::LEDconfig(uint8_t brightness,
									 uint16_t cycleTime,
									 uint16_t offTime,
									 uint8_t granularity){
	return false;

}

bool QwiicButton::LEDoff(){
	return false;

}

bool QwiicButton::LEDon(uint8_t brightness ){
	return false;

}

