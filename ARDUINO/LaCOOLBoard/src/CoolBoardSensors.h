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


#ifndef CoolBoardSensors_H
#define CoolBoardSensors_H

#include "Arduino.h"
#include "internals/CoolSI114X.h"        // Light sensor Support
#include "internals/CoolSparkFunBME280.h"// Environmental sensor Support

/**
*
*	\class CoolBoardSensors 
*	\brief This class handles the On-Board Sensors. 
* 
*
*/
class CoolBoardSensors
{

public:

	//Constructor
	CoolBoardSensors();

	void begin();

	//data is in json
	String read();

	void allActive();

	//additional method
	void end();

	bool config();

	void printConf();

	//environment sensor methods

	//set the enviornment sensor settings , if argument is ommitted , default value will be assigned
	void setEnvSensorSettings( uint8_t commInterface=I2C_MODE, uint8_t I2CAddress=0x76,    uint8_t runMode = 3,
					   
				   uint8_t tStandby=0	, uint8_t filter=0,    uint8_t tempOverSample=1,                          					   
				   uint8_t pressOverSample= 1,    uint8_t humidOverSample= 1);

	//VBat
	float readVBat();

	//Moisture

	float readMoisture();

	//sensor objects :
	/**
	*	SI114X light sensor instance
	*/
	CoolSI114X lightSensor;				
	
	/**
	*	BME280 environment sensor instance
	*/
	BME280 envSensor;			        
	
private:
	/**
	*	lightActive structure
	*
	*	set visible to 1 to have visibleLight Readings
	*
	*	set ir to 1 to have infraRed Readings
	*
	*	set uv to 1 to have ultraViolet Readings
	*/	
	struct lightActive
	{
		bool visible=0;
		bool ir=0;
		bool uv=0;	

	}lightDataActive;

	/**
	*	airActive structure
	*
	*	set temperature to 1 to have temperature Readings
	*
	*	set humidity to 1 to have humidity Readings
	*
	*	set pressure to 1 to have pressure Readings
	*/	
	struct airActive
	{
		bool temperature=0;
		bool humidity=0;
		bool pressure=0;

	}airDataActive;

	/**
	*	 Moisture Enable Pin
	*/
	const int EnMoisture = 13;                      

	/**
	*	Analog Multiplexer  LOW=Vbat , HIGH=Moisture
	*/
	const int AnMplex = 12;                        

	/**
	*	set vbatActive to 1 to have battery voltage Readings
	*/
        bool vbatActive=0;

	/**
	*	set soilMoistureActive to 1 to have soil Moisture Readings
	*/
	bool soilMoistureActive=0;

};

#endif
