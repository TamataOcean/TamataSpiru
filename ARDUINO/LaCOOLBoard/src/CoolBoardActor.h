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

#ifndef CoolBoardActor_H
#define CoolBoardActor_H

#include "Arduino.h"


/**
*	\class CoolBoardActor
*	\brief This class manages the CoolBoardActor 
*
*/
class CoolBoardActor
{
public:

	void begin();		       //starts the CoolBoardActor

 	void write(bool action);			//CoolBoardActor
 
	String doAction(const char* data, int hour, int minute );

	void normalAction(float measurment);

	void invertedAction(float measurment);

	void temporalActionOff();

	void temporalActionOn();

	void mixedTemporalActionOff(float measurment);

	void mixedTemporalActionOn(float measurment);

	void hourAction( int hour); 

	void mixedHourAction(int hour, float measurment);

	void minuteAction(int minute);

	void mixedMinuteAction(int minute,float measurment);

	void hourMinuteAction(int hour,int minute);

	void mixedHourMinuteAction(int hour,int minute ,float measurment);

        bool config();

	void printConf();

	/**
	*	pin Number value
	*	the only available pin that can be used
	*	to run a small actor is pin 15
	*/
	const int pin = 15; //pin for the actor

//private:
	
	/**
	*	the Actor's state
	*/
	struct state{
		
		/**
		*	actif flag
		*	set to 1 when using an actor
		*	set to 0 otherwise
		*/
		bool actif=0;

		/**
		*	temporal flag
		*	set to 1 to declare a temporal actor
		*	set to 0 otherwise
		*/
		bool temporal=0;

		/**
		*	inverted flag
		*	set to 1 to declare an inverted actor
		*	set to 0 to declare a normal actor
		*/
		bool inverted=0;
		
		/**
		*	"type":["temperature","hour"]
		*
		*	the primary type is related to the sensor's type (type[0])
		*
		*	if both types are present and valid, the actor is a mixedActor
		*/	
		String primaryType="";

		/**
		*	"type":["temperature","hour"]
		*
		*	the secondary type if present is hour or minute or hourMinute (type[1])
		*
		*	if both types are present and valid, the actor is a mixedActor
		*/
		String secondaryType="";  
		
		/**
		*	"low":[20,5000,18,10]
		*
		*	rangeLow : this is the minimum at which the actor becomes actif (low[0])
		*
		*/
		int rangeLow=0;

		/**
		*	"low":[20,5000,18,10]
		*
		*	timeLow : this is the time the actor is off in temporal mode (low[1]) (ms)
		*/
		unsigned long timeLow=0;
		
		/**
		*	"low":[20,5000,18,10]
		*
		*	hour low :this is the hour when to turn off the actor in temporal/hour(hourMinute) mode (low[2] )		
		*/		
		int hourLow=0;
 
		/**
		*	"low":[20,5000,18,10]
		*
		*	minute low :this is the minute when to turn off the actor in temporal/minute(hourMinute) mode (low[3])
		*/
		int minuteLow=0;


		/**
		*	"high":[30,2000,17,1]
		*	
		*	rangeHigh : this is the maximum at which the actor becomes inactif(high[0])
		*/
		int rangeHigh=0;
		
		/**
		*	"high":[30,2000,17,1]
		*
		*	timeHigh : this is the time the actor is on in temporal mode(high[1]) (ms)
		*/
		unsigned long timeHigh=0;
		
		/**
		*	"high":[30,2000,17,1]
		*
		*	hourHigh : this is the hour when to turn on the actor in temporal/hour(hourMinute) mode(high[2])
		*/
		int hourHigh=0; 

		/**
		*	"high":[30,2000,17,1]
		*		
		*	minuteHigh : this is the minute when to turn on the actor in temporal/minute(hourMinute) mode (high[3]) 
		*/
		int minuteHigh=0;

		/**
		*	actifTime : period of Time spent actif , used in Temporal mode
		*	in ms
		*/
		unsigned long actifTime=0;
	
		/**
		*	inactifTime : period of Time spent inactif , used in Temporal mode
		*	in ms
		*/
		unsigned long inactifTime=0;
 
		/**
		*	failsave : Switches of the actor if a metric rises above high treshold in mixed mode
		*/
		bool failsave = false ;
	
	}actor;
	
};

#endif
