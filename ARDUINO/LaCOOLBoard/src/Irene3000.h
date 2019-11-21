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


#ifndef IRENE3000_H
#define IRENE3000_H
#include "internals/CoolAdafruit_ADS1015.h"   

#include <Arduino.h>


#define ADC_MAXIMUM_VALUE     32767
#define REFERENCE_VOLTAGE_GAIN_4  1.024
#define V_GAIN_2 0.0625
#define V_GAIN_4 0.03125
#define V_GAIN_8 0.015625
#define Write_Check      0x1234

#define button 0
#define temp 1
#define freeAdc 2
#define ph 3

/**
*	\class Irene3000
*	\brief This class is provided to manage
*	the Irene3000 Ph/Temperature Shield
*
*/
class Irene3000
{
public:

	void startADC();
	
	void begin();

	bool config();

	bool ireneButton();

	void printConf();

	String read();

	int readButton();

	void setGain(adsGain_t gain);

	int readADSChannel2(adsGain_t gain);

	float readPh( double t );

	double readTemp();

	void resetParams();

	void calibratepH7();

	void calibratepH4();

	void calcpHSlope ();

	adsGain_t gainConvert(uint16_t tempGain);
	
	bool saveParams();


private:
	
	/**
	*	ADS1115 instance
	*/
	Adafruit_ADS1115 ads;                                                                                           

	/**
	*	Led handler instance
	*/
	CoolBoardLed coolBoardLed;

	/**
	*	PH Sensor Calibration parameters
	*
	*	pH7Cal : PH 7 Calibration value
	*		
	*	pH4Cal : PH 4 Calibration value
	*
	*	pHStep : PH Slope Step value
	*	
	*/
	struct parameters_T
	{
		unsigned int WriteCheck=0;
		int pH7Cal, pH4Cal=0;
		float pHStep=1;
	}params;

	/**
	*	Irene sensors state struct
	*
	*	active : if the related sensor is active or not
	*		
	*	gain : what is the used gain for the related sensor (used only for the free channel)
	*
	*	type : the type of the used sensor(used only for the free channel )
	*	
	*/
	struct state
	{
		bool active=0;
		adsGain_t gain=GAIN_TWOTHIRDS;
		String type="";

	} waterTemp, phProbe,adc2;

	/**
	*	vRef constant used for ADC conversion
	*/
	const float vRef = 1.024;                                                            
	
	/**
	*	opAmpGain constant used for ADC conversion
	*/
	const float opampGain = 5.25;                               

};


#endif
