//
//  QwiicButton.hpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/21/21.
//

#ifndef QwiicButton_hpp
#define QwiicButton_hpp

#include <stdio.h>
#include "I2C.hpp"


using namespace std;

class QwiicButton
{
 
public:
	QwiicButton();
	~QwiicButton();
 
	// Address of QwiicButton (0x6E,0x6F)
 	bool begin(uint8_t deviceAddress = 0x6F);
	bool begin(uint8_t deviceAddress,  int &error);
 
	void stop();
	bool isOpen();
 
	uint8_t	getDevAddr();

	bool isPressed();                       //Returns 1 if the button/switch is pressed, and 0 otherwise

	
	bool LEDconfig(uint8_t brightness, uint16_t cycleTime,
						uint16_t offTime, uint8_t granularity = 1); //Configures the LED with the given max brightness, granularity (1 is fine for most applications), cycle time, and off time.
	bool LEDoff();                                             //Turns the onboard LED off
	bool LEDon(uint8_t brightness = 255);                      //Turns the onboard LED on with specified brightness. Set brightness to an integer between 0 and 255, where 0 is off and 255 is max brightness.

private:
	
	I2C 		_i2cPort;
	bool		_isSetup;

};

#endif /* QwiicButton_hpp */