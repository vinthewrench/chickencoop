//
//  QwiicButton.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/21/21.
//

#include "QwiicButton.hpp"
#include "LogMgr.hpp"

//Register Pointer Map
enum Qwiic_Button_Register : uint8_t
{
	 ID = 0x00,
	 FIRMWARE_MINOR = 0x01,
	 FIRMWARE_MAJOR = 0x02,
	 BUTTON_STATUS = 0x03,
	 INTERRUPT_CONFIG = 0x04,
	 BUTTON_DEBOUNCE_TIME = 0x05,
	 PRESSED_QUEUE_STATUS = 0x07,
	 PRESSED_QUEUE_FRONT = 0x08,
	 PRESSED_QUEUE_BACK = 0x0C,
	 CLICKED_QUEUE_STATUS = 0x10,
	 CLICKED_QUEUE_FRONT = 0x11,
	 CLICKED_QUEUE_BACK = 0x15,
	 LED_BRIGHTNESS = 0x19,
	 LED_PULSE_GRANULARITY = 0x1A,
	 LED_PULSE_CYCLE_TIME = 0x1B,
	 LED_PULSE_OFF_TIME = 0x1D,
	 I2C_ADDRESS = 0x1F,
};

typedef union {
	 struct
	 {
		  bool eventAvailable : 1; //This is bit 0. User mutable, gets set to 1 when a new event occurs. User is expected to write 0 to clear the flag.
		  bool hasBeenClicked : 1; //Defaults to zero on POR. Gets set to one when the button gets clicked. Must be cleared by the user.
		  bool isPressed : 1;      //Gets set to one if button is pushed.
		  bool : 5;
	 };
	 uint8_t byteWrapped;
} statusRegisterBitField;

typedef union {
	 struct
	 {
		  bool clickedEnable : 1; //This is bit 0. user mutable, set to 1 to enable an interrupt when the button is clicked. Defaults to 0.
		  bool pressedEnable : 1; //user mutable, set to 1 to enable an interrupt when the button is pressed. Defaults to 0.
		  bool : 6;
	 };
	 uint8_t byteWrapped;
} interruptConfigBitField;

typedef union {
	 struct
	 {
		  bool popRequest : 1; //This is bit 0. User mutable, user sets to 1 to pop from queue, we pop from queue and set the bit back to zero.
		  bool isEmpty : 1;    //user immutable, returns 1 or 0 depending on whether or not the queue is empty
		  bool isFull : 1;     //user immutable, returns 1 or 0 depending on whether or not the queue is full
		  bool : 5;
	 };
	 uint8_t byteWrapped;
} queueStatusBitField;


QwiicButton::QwiicButton(){
	_deviceAddress = 0x00;

}

QwiicButton::~QwiicButton(){
	stop();
	
}

bool QwiicButton::begin(uint8_t deviceAddress){
	int error = 0;

	return begin(deviceAddress, error);
}

 
bool QwiicButton::begin(uint8_t deviceAddress,   int &error){

	bool status = true;

	uint8_t  deviceType = 0;
	_deviceAddress = deviceAddress;
	
	if(getDeviceType(deviceType) && deviceType != 0x5d) {
		LOGT_INFO("QwiicButton(%02x) unexpected device type = %02x\n", deviceAddress, deviceType );
		error = ENODEV;
		status = false;
	}
	
	return status;
}
 
void QwiicButton::stop(){
//	LOG_INFO("QwiicButton(%02x) stop\n",  _i2cPort.getDevAddr());
}
 

uint8_t	QwiicButton::getDevAddr(){
	return _deviceAddress;
};


bool QwiicButton::isPressed(bool &state)
{
	bool success = false;

	statusRegisterBitField buttonStatus = {.byteWrapped = 0};
	I2C i2cPort;

	if( i2cPort.begin(_deviceAddress)) {
		i2cPort.readBytes(BUTTON_STATUS, &buttonStatus.byteWrapped, 1);
		i2cPort.stop();
		state =  buttonStatus.isPressed;
		success = true;
	}

	return success;
}

bool QwiicButton::hasBeenClicked(bool &state)
{
	bool success = false;
	
	statusRegisterBitField buttonStatus = {.byteWrapped = 0};
	I2C i2cPort;
	
	if( i2cPort.begin(_deviceAddress)) {
		i2cPort.readBytes(BUTTON_STATUS, &buttonStatus.byteWrapped, 1);
		i2cPort.stop();
		
		state =  buttonStatus.hasBeenClicked;
		success = true;
		
	}
	return success;
	
}


bool QwiicButton::clearEventBits()
{
	bool success = false;
	I2C i2cPort;

	if( i2cPort.begin(_deviceAddress)) {
		
		statusRegisterBitField buttonStatus = {.byteWrapped = 0};
		statusRegisterBitField buttonStatus1 = {.byteWrapped = 0};
		
		if(i2cPort.readBytes(BUTTON_STATUS, &buttonStatus.byteWrapped, 1) == 1) {
			
			buttonStatus.isPressed = 0;
			buttonStatus.hasBeenClicked = 0;
			buttonStatus.eventAvailable = 0;
			
			bool success = i2cPort.writeByte(BUTTON_STATUS, buttonStatus.byteWrapped);
			success &= (i2cPort.readBytes(BUTTON_STATUS, &buttonStatus1.byteWrapped, 1) != 1);
			success &= (buttonStatus1.byteWrapped == buttonStatus.byteWrapped);
		}
		i2cPort.stop();
	}
	
	return success;
}


 
bool QwiicButton::LEDconfig(uint8_t brightness,
									 uint16_t cycleTime,
									 uint16_t offTime,
									 uint8_t granularity){
	
	bool success = false;
	I2C i2cPort;

	if( i2cPort.begin(_deviceAddress)) {
		
		bool success = i2cPort.writeByte(LED_BRIGHTNESS, brightness);
		success &= i2cPort.writeByte(LED_PULSE_GRANULARITY, granularity);
		success &= i2cPort.writeWord(LED_PULSE_CYCLE_TIME, cycleTime);
		success &= i2cPort.writeWord(LED_PULSE_OFF_TIME, offTime);
		
		i2cPort.stop();
	}
	
	return success;
}

bool QwiicButton::LEDoff(){
	return LEDconfig(0, 0, 0);
}

bool QwiicButton::LEDon(uint8_t brightness ){
	return LEDconfig(brightness, 0, 0);
}

bool QwiicButton::getDeviceType(uint8_t &deviceType){
	
	bool success = false;
	I2C i2cPort;

	if( i2cPort.begin(_deviceAddress)) {
		
		uint8_t registerByte[1] = {0};
		if(i2cPort.readBytes(ID, registerByte, 1) == 1){
			deviceType = registerByte[0];
			success = true;
		}
		
		i2cPort.stop();
	}
	
	return success;
}
 
bool QwiicButton::getFirmwareVersion(uint16_t & version){
	
	bool success = false;
	I2C i2cPort;

	if( i2cPort.begin(_deviceAddress)) {
		
		uint8_t registerBytes[2] = {0,0};
		
		if(i2cPort.readBytes(FIRMWARE_MINOR, registerBytes, 2) != 2){
			version = ((registerBytes[1]) << 8) | (registerBytes[0]);
			success = true;
		}
		
		i2cPort.stop();
	}
	
	return success;
}

