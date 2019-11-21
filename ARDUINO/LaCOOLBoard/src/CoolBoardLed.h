/**
*	Copyright (c) 2018 La Cool Co SAS
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files (the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included in all
*	copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*	SOFTWARE.
*
*/

#ifndef CoolOnBoardLed_H
#define CoolOnBoardLed_H

#include"Arduino.h"

#include <NeoPixelBus.h>


/**
*	\class	CoolBoardLed
*	\brief	This class handles the led in the Sensor Board
*/
class CoolBoardLed

{

public:

	void begin();			 


	void write(int R, int G, int B);       

	void end();				//delete the dynamic led;

	bool config();

	void activate();

	void printConf();


	//Neo Pixel Led methods :
	void fade(int R, int G, int B, float T);

	void blink(int R, int G, int B, float T);

	void fadeIn(int R, int G, int B, float T);

	void fadeOut(int R, int G, int B, float T);

	void strobe(int R, int G, int B, float T); 



private:

	/**
	*	NeoPixel Led Instance
	*/
	NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* neoPixelLed = NULL; //template instance must be dynamic

	/**
	*	ledActive flag 
	*	set to 1 if you want LED animation	
	*/
	bool ledActive=0;

};

#endif
