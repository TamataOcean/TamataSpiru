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

#ifndef CoolBoard_H
#define CoolBoard_H


#include"CoolFileSystem.h" 	//CoolBoard File System Manager
#include"CoolBoardSensors.h"	//CoolBoard Sensor Board Manager
#include"CoolBoardLed.h"	//CoolBoard Led Manager
#include"CoolTime.h"		//CoolBoard Real Time Clock Manager
#include"CoolMQTT.h"		//CoolBoard MQTT Manager
#include"Jetpack.h"		//CoolBoard Jetpack Manager
#include"Irene3000.h"		//CoolBoard Irene3000 Manager
#include"ExternalSensors.h"	//CoolBoard External Sensors Manager
#include"CoolWifi.h"		//CoolBoard Wifi Manager
#include"CoolBoardActor.h"	//CoolBoard Actor Manager

#include"Arduino.h"		//Arduino Defs

/**
*	\class	CoolBoard
*	
*	\brief This class manages the CoolBoard and all of
*	Its functions
*/
class CoolBoard
{

public:
	//Constructor
	CoolBoard();

	void begin(); 
	
	bool config();

	void update(const char*answer );

	void offLineMode();

	void onLineMode();

	int connect();
	
	int isConnected();

	unsigned long getLogInterval();

	void printConf();

	void sleep(unsigned long interval);

	String readSensors();
	
	void initReadI2C();
	
	String userData();

	bool sendConfig(const char* moduleName, const char* filePath);


private:

	/**
	*	fileSystem handler instance
	*/
	CoolFileSystem fileSystem; 

	/**
	*	Sensor Board handler instance
	*/
	CoolBoardSensors coolBoardSensors;

	/**
	*	Led handler instance
	*/
	CoolBoardLed coolBoardLed;

	/**
	*	RTC handler instance
	*/
	CoolTime rtc;
	
	/**
	*	Wifi handler instance
	*/
	CoolWifi wifiManager;

	/**
	*	MQTT handler instance
	*/
	CoolMQTT mqtt;

	/**
	*	Jetpack handler instance
	*/
	Jetpack jetPack;

	/**
	*	Irene3000 handler instance
	*/
	Irene3000 irene3000;

	/**
	*	External Sensors handler instance
	*/
	ExternalSensors externalSensors;
	
	/**
	*	On Board Actor handler instance
	*/
	CoolBoardActor	onBoardActor;

	/**
	*	userActive flag,
	*	set to 1 to collect userData(MAC,userName,TimeStamp) 
	*/
	bool userActive=0;

	/**
	*	ireneActive flag,
	*	set to 1 when using an Irene module
	*/
	bool ireneActive=0;

	/**
	*	jetpackActive flag,
	*	set to 1 when using a Jetpack module
	*/
	bool jetpackActive=0;

	/**
	*	externalSensors flag,
	*	set to 1 when using 1/many external Sensor(s)
	*/
	bool externalSensorsActive=0;		

	/**
	*	sleepActive flag,
	*	set to 1 when using sleep Mode
	*	in Sleep mode : the CoolBoard will do
	*	a cycle (init, read sensors, do action,log)
	*	and go to sleep for a LogInterval period of time
	*/
	bool sleepActive=0;
	
	/**
	*	manual flag,
	*	set to 1 when using manual mode
	*	in manual Mode , user can activate/deactivate
	*	actors through a specific MQTT command
	*
	*	/!\ in manual Mode, receving an update will not reset the CoolBoard
	*	/!\ resetting the CoolBoard in manual mode will deactivate all actors 
	*/
	bool manual=0;	

	/**
	*	saveAsJSON Flag,
	*	save data as JSON String when in offLineMode
	*	ATTENTION THIS CONSUMES MUCH MORE MEMORY 
	*	THEN SAVING DATA AS CSV
	*/
	bool saveAsJSON=1;

	/**
	*	saveAsCSV Flag,
	*	save data in a CSV file when in offLineMode
	*	BEST FOR OFF GRID SOLUTIONS!!!
	*/
	bool saveAsCSV=1;

	/**
	*	log Interval value,
	*	the period of time between logs
	-	in Seconds
	*/
	unsigned long logInterval=1;

	/**
	*	last time the Client sent a Message over MQTT
	*	in ms
	*/
	unsigned long previousLogTime=0;

	/**
	*	data string,
	*	string that contains sensors data
	*/
	String data="";

	/**
	*	answer string,
	*	string that contains received MQTT messages
	*/
	String answer="";

	/**
	*	Enable I2C pin,
	*	double usage for I2C and shift register latch , HIGH=I2C , LOW=shift register latch
	* 	All I2C is over pins (2,14)
	*/
	const int EnI2C = 5;                            


};

#endif
