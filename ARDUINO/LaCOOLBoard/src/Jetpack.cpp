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


#include "FS.h"
#include "Arduino.h"
#include "ArduinoJson.h"

#include "Jetpack.h"


#define DEBUG 1


/**
*	Jetpack::begin():
*	This method is provided to
*	initialise the pin that control
*	the Jetpack shield
*/
void Jetpack::begin()
{

#if DEBUG == 1 
 
	Serial.println( F("Entering Jetpack.begin() ") );
	Serial.println();

#endif

	pinMode(EnI2C,OUTPUT);
	pinMode(dataPin,OUTPUT);
	pinMode(clockPin,OUTPUT);
	
	

}


/**
*	Jetpack::write(action):
*	This method is provided to write
*	the given action to the entire Jetpack
*	action is a Byte (8 bits ), each bit goes 
*	to an output. 
*	MSBFirst 
*/
void Jetpack::write(byte action)
{

#if DEBUG == 1

	Serial.println( F("Entering Jetpack.write()") );
	Serial.println();

	Serial.println( F("writing this action : ") );
	Serial.println(action,BIN);
	Serial.println();

#endif 

	this->action=action;

	
	digitalWrite(EnI2C, LOW);
	
	shiftOut(dataPin, clockPin, MSBFIRST, this->action);

	digitalWrite(EnI2C, HIGH);


}
	
/**
*	Jetpack::writeBit(pin,state):
*	This method is provided to write
*	the given state to the given pin
*/
void Jetpack::writeBit(byte pin,bool state) 
{

#if DEBUG == 1 

	Serial.println( F("Entering Jetpack.writeBit() ") );

	Serial.print( F("Writing ") );
	Serial.print(state);

	Serial.print( F("to pin N°") );
	Serial.print(pin);

	Serial.println();

#endif

	bitWrite(this->action, pin, state);
	digitalWrite(EnI2C, LOW);
	
	shiftOut(dataPin, clockPin, MSBFIRST, this->action);

	digitalWrite(EnI2C, HIGH);

}

/**
*	Jetpack::doAction(sensor data ):
*	This method is provided to automate the Jetpack.
*	
*	The result action is the result of
*	checking the different flags of an actor
*	(actif , temporal ,inverted, primaryType
*	and secondaryType ) and the corresponding
*	call to the appropriate helping method
*
*	\return a string of the current Jetpack state
*	
*/
String Jetpack::doAction( const char* data, int hour, int minute )
{

#if DEBUG == 1 

	Serial.println( F("Entering Jetpack.doAction()") );
	Serial.println();

	Serial.println( F("input data is :") );
	Serial.println(data);
	Serial.println();

	Serial.print( F("Hour from Coolboard.cpp : "));
	Serial.println(hour);
	Serial.print( F("Minute from Coolboard.cpp : "));
	Serial.println(minute);

#endif 

	//input json buffer and object
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(data);
	
	//output json buffer and object
	String jetpackState;
	DynamicJsonBuffer  jsonBufferOutput ;
	JsonObject& rootOutput = jsonBufferOutput.createObject();
	
	if (!root.success()) 
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to parse json object ") );
		Serial.println();
	
	#endif 

	}
	else if(!rootOutput.success())
	{
	
	#if DEBUG == 1 
		
		Serial.println(F("failed to create output json object"));	
		Serial.println();
	#endif	
	
	}
	else
	{
	
	#if DEBUG == 1 

		Serial.println( F("created Json object :") );
		root.printTo(Serial);
		Serial.println();

		Serial.print(F("jsonBuffer size: "));
		Serial.println(jsonBuffer.size());
		Serial.println();

	
	#endif 

		//invert the current action state for each actor
		//if the value is outside the limits
		for(int i=0;i<8;i++)
		{
			//----- Special Action Spiru ----------
			//-------------------------------------
			bool SpecialSpiruActor = false;
			if(this->actors[i].secondaryType=="heatSpiru") {
				this->heatSpiru(i, hour, root[this->actors[i].primaryType].as<float>());
				SpecialSpiruActor = true;
			}	
			else if(this->actors[i].secondaryType=="bublerSpiru") 
			{
				this->bublerSpiru(i, hour);
				SpecialSpiruActor = true;
			}
			else if(this->actors[i].secondaryType=="perilstaticSpiru") {
				this->perilstaticSpiru(i,hour);
				SpecialSpiruActor = true;
			}


			//check if actor is actif
			if(this->actors[i].actif==1 && !SpecialSpiruActor )
			{
				//normal actor
				if(this->actors[i].temporal == 0)
				{
					//not inverted actor
					if(this->actors[i].inverted==0)
					{
						this->normalAction(i,root[this->actors[i].primaryType].as<float>());
					}
					//inverted actor
					else if(this->actors[i].inverted==1)
					{
						this->invertedAction(i,root[this->actors[i].primaryType].as<float>());			
					}
				}

				//temporal actor
				else if(this->actors[i].temporal == 1 )
				{
					//hour actor
					if(this->actors[i].secondaryType=="hour")
					{
						//mixed hour actor
						if(root[this->actors[i].primaryType].success() )
						{
							this->mixedHourAction(i, hour, root[this->actors[i].primaryType].as<float>());
						}
						//normal hour actor
						else
						{
							this->hourAction(i, hour);
						}
					
					}
					//minute actor
					else if(this->actors[i].secondaryType=="minute")
					{
						//mixed minute actor
						if(root[this->actors[i].primaryType].success() )
						{
							this->mixedMinuteAction(i,root[this->actors[i].secondaryType].as<int>(),root[this->actors[i].primaryType].as<float>());
						}
						//normal minute actor
						else
						{
							this->minuteAction(i,root[this->actors[i].secondaryType].as<int>());
						}
					}
					//hourMinute actor
					else if(this->actors[i].secondaryType=="hourMinute")
					{
						//mixed hourMinute actor
						if(root[this->actors[i].primaryType].success() )
						{
							this->mixedHourMinuteAction(i, hour, minute,root[this->actors[i].primaryType].as<float>());
						}
						//normal hourMinute actor
						else
						{
							this->hourMinuteAction(i, hour, minute);
						}
					}
					//normal temporal actor
					else if(this->actors[i].secondaryType=="")
					{
						//mixed temporal actor
						if(root[this->actors[i].primaryType].success() )
						{
							this->mixedTemporalActionOn(i,root[this->actors[i].primaryType].as<float>());
						}
						//normal temporal actor
						else
						{
							this->temporalActionOff(i);
						}
											
					}

				}
			}
			//inactif actor
			else if(this->actors[i].actif == 0 && !SpecialSpiruActor)
			{
				//temporal actor
				if(this->actors[i].temporal==1)
				{
					//mixed temporal actor
					if(root[this->actors[i].primaryType].success() )
					{
						this->mixedTemporalActionOff(i,root[this->actors[i].primaryType].as<float>());
					}
					//normal temporal actor
					else
					{
						this->temporalActionOn(i);
					}
				}			
			}
			/*if(this->action == 1)
			{
				rootOutput[String("Act")+String(i)]=true;
			}
			else rootOutput[String("Act")+String(i)]=false;*/

			rootOutput[String("Act") + String(i)] = (bitRead(this->action, i) == 1);
		}

		this->write(this->action);

	}
	
	rootOutput.printTo(jetpackState);

	return(jetpackState); 
}

/**
*	Jetpack::config():
*	This method is provided to configure the
*	Jetpack with a configuration file
*
*	\return true if successful,false otherwise
*/ 
bool Jetpack::config()
{

#if DEBUG == 1 

	Serial.println( F("Entering Jetpack.config() ") );
	Serial.println();

#endif

	File jetPackConfig = SPIFFS.open("/jetPackConfig.json", "r");

	if (!jetPackConfig) 
	{

	#if DEBUG == 1 

		Serial.println( F("failed to read /jetPackConfig.json ") );
		Serial.println();

	#endif

		return(false);
	}
	else
	{
		size_t size = jetPackConfig.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		jetPackConfig.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{
		
		#if DEBUG == 1 

			Serial.println( F("failed to parse jetpack config json from file ") );
			Serial.println();

		#endif

			return(false);
		} 
		else
		{ 
 		
		#if DEBUG == 1 

			Serial.println( F("read configuration file : ") );
			json.printTo(Serial);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();

		
		#endif
  
			for(int i=0;i<8;i++)
			{	
				if(json[String("Act")+String(i)].success())
				{
					//parsing actif key
					if(json[String("Act")+String(i)]["actif"].success() )
					{
						this->actors[i].actif=json[String("Act")+String(i)]["actif"];
					}
					else
					{
						this->actors[i].actif=this->actors[i].actif;
					}
					json[String("Act")+String(i)]["actif"]=this->actors[i].actif;
					
					//parsing temporal key
					if(json[String("Act")+String(i)]["temporal"].success() )
					{
						this->actors[i].temporal=json[String("Act")+String(i)]["temporal"];
					}
					else
					{
						this->actors[i].temporal=this->actors[i].temporal;
					}
					json[String("Act")+String(i)]["temporal"]=this->actors[i].temporal;
					
					//parsing inverted key
					if(json[String("Act")+String(i)]["inverted"].success() )
					{
						this->actors[i].inverted=json[String("Act")+String(i)]["inverted"];
					}
					else
					{
						this->actors[i].inverted=this->actors[i].inverted;
					}
					json[String("Act")+String(i)]["inverted"]=this->actors[i].inverted;

					//parsing inverted key
					if(json[String("Act")+String(i)]["inverted"].success() )
					{
						this->actors[i].inverted=json[String("Act")+String(i)]["inverted"];
					}
					else
					{
						this->actors[i].inverted=this->actors[i].inverted;
					}
					json[String("Act")+String(i)]["inverted"]=this->actors[i].inverted;
					
					//parsing low key
					if(json[String("Act")+String(i)]["low"].success() )
					{
						this->actors[i].rangeLow=json[String("Act")+String(i)]["low"][0];
						this->actors[i].timeLow=json[String("Act")+String(i)]["low"][1];
						this->actors[i].hourLow=json[String("Act")+String(i)]["low"][2];						
						this->actors[i].minuteLow=json[String("Act")+String(i)]["low"][3];						
					}
					else
					{
						this->actors[i].rangeLow=this->actors[i].rangeLow;
						this->actors[i].timeLow=this->actors[i].timeLow;
						this->actors[i].hourLow=this->actors[i].hourLow;
						this->actors[i].minuteLow=this->actors[i].minuteLow;						
					}
					json[String("Act")+String(i)]["low"][0]=this->actors[i].rangeLow;
					json[String("Act")+String(i)]["low"][1]=this->actors[i].timeLow;
					json[String("Act")+String(i)]["low"][2]=this->actors[i].hourLow;
					json[String("Act")+String(i)]["low"][3]=this->actors[i].minuteLow;

					//parsing high key
					if(json[String("Act")+String(i)]["high"].success() )
					{
						this->actors[i].rangeHigh=json[String("Act")+String(i)]["high"][0];
						this->actors[i].timeHigh=json[String("Act")+String(i)]["high"][1];
						this->actors[i].hourHigh=json[String("Act")+String(i)]["high"][2];						
						this->actors[i].minuteHigh=json[String("Act")+String(i)]["high"][3];						
					}
					else
					{
						this->actors[i].rangeHigh=this->actors[i].rangeHigh;
						this->actors[i].timeHigh=this->actors[i].timeHigh;
						this->actors[i].hourHigh=this->actors[i].hourHigh;
						this->actors[i].minuteHigh=this->actors[i].minuteHigh;
					}
					json[String("Act")+String(i)]["high"][0]=this->actors[i].rangeHigh;
					json[String("Act")+String(i)]["high"][1]=this->actors[i].timeHigh;
					json[String("Act")+String(i)]["high"][2]=this->actors[i].hourHigh;
					json[String("Act")+String(i)]["high"][3]=this->actors[i].minuteHigh;

					//parsing type key
					if(json[String("Act")+String(i)]["type"].success() )
					{
						this->actors[i].primaryType=json[String("Act")+String(i)]["type"][0].as<String>();
						this->actors[i].secondaryType=json[String("Act")+String(i)]["type"][1].as<String>();						
						
					}
					else
					{
						this->actors[i].primaryType=this->actors[i].primaryType;
						this->actors[i].secondaryType=this->actors[i].secondaryType;
					}

					//parsing comment key
					if(json[String("Act")+String(i)]["comment"].success() )
					{
						this->actors[i].comment=json[String("Act")+String(i)]["comment"].as<String>();						
					}
					else {
						//json[String("Act")+String(i)]["comment"] = this->actors[i].comment;
						this->actors[i].comment=this->actors[i].comment;	
					}

					json[String("Act")+String(i)]["type"][0]=this->actors[i].primaryType;
					json[String("Act")+String(i)]["type"][1]=this->actors[i].secondaryType;
				}
				else
				{
					this->actors[i]=this->actors[i];
				}
				
				json[String("Act")+String(i)]["actif"]=this->actors[i].actif;
				json[String("Act")+String(i)]["temporal"]=this->actors[i].temporal;
				json[String("Act")+String(i)]["inverted"]=this->actors[i].inverted;

				json[String("Act")+String(i)]["low"][0]=this->actors[i].rangeLow;
				json[String("Act")+String(i)]["low"][1]=this->actors[i].timeLow;
				json[String("Act")+String(i)]["low"][2]=this->actors[i].hourLow;
				json[String("Act")+String(i)]["low"][3]=this->actors[i].minuteLow;

				json[String("Act")+String(i)]["high"][0]=this->actors[i].rangeHigh;
				json[String("Act")+String(i)]["high"][1]=this->actors[i].timeHigh;
				json[String("Act")+String(i)]["high"][2]=this->actors[i].hourHigh;
				json[String("Act")+String(i)]["high"][3]=this->actors[i].minuteHigh;

				json[String("Act")+String(i)]["type"][0]=this->actors[i].primaryType;
				json[String("Act")+String(i)]["type"][1]=this->actors[i].secondaryType;



			}
			

			jetPackConfig.close();			
			jetPackConfig = SPIFFS.open("/jetPackConfig.json", "w");			
			if(!jetPackConfig)
			{
			
			#if DEBUG == 1 

				Serial.println(F("failed to write to /jetPackConfig.json ") );
				Serial.println();
			
			#endif
				
				return(false);			
			}  

			json.printTo(jetPackConfig);
			jetPackConfig.close();

		#if DEBUG == 1 
			
			Serial.println(F("saved configuration : "));
			json.printTo(Serial );
			Serial.println();		
		
		#endif

			return(true); 
		}
	}	
	

}

/**
*	Jetpack::printConf():
*	This method is provided to
*	print the configuration to the 
*	Serial Monitor
*/
void Jetpack::printConf()
{

#if DEBUG == 1 

	Serial.println( F("Enter Jetpack.printConf() ") );
	Serial.println();

#endif 
	Serial.println(F( "Jetpack configuration " ) ) ;
 
        for(int i=0;i<8;i++)
	{	
		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" actif :"));
		Serial.println(this->actors[i].actif);
		
		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" comment :"));
		Serial.println(this->actors[i].comment);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" temporal :"));
		Serial.println(this->actors[i].temporal);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" inverted :"));
		Serial.println(this->actors[i].inverted);


		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" primary Type :"));
		Serial.println(this->actors[i].primaryType);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" secondary Type :"));		
		Serial.println(this->actors[i].secondaryType);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" range Low :"));
		Serial.println(this->actors[i].rangeLow);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" time Low :"));
		Serial.println(this->actors[i].timeLow);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" hour low:"));
		Serial.println(this->actors[i].hourLow);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" minute low:"));
		Serial.println(this->actors[i].minuteLow);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" range High:"));
		Serial.println(this->actors[i].rangeHigh);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" time High:"));
		Serial.println(this->actors[i].timeHigh);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" hour high:"));
		Serial.println(this->actors[i].hourHigh);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" minute high:"));
		Serial.println(this->actors[i].minuteHigh);

		Serial.println(); 

	}

	Serial.println();
}

/**
*	Jetpack::normalAction(actorNumber , measured value):
*	This method is provided to
*	handle normal actors.
*	it changes the action according to wether the
*	measured value is: > rangeHigh ( deactivate actor)
*	or < rangeLow (activate actor )
*/
void Jetpack::normalAction(int actorNumber,float measurment)
{

#if DEBUG == 1
	
	Serial.print(F("none inverted Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment: "+this->actors[actorNumber].comment);

	Serial.print(F("measured value : "));
	Serial.println(measurment);

	Serial.print(F("high range : "));
	Serial.println(this->actors[actorNumber].rangeHigh);

	Serial.print(F("low range : "));
	Serial.println(this->actors[actorNumber].rangeLow);

#endif

	//measured value lower than minimum range : activate actor
	if(measurment < this->actors[actorNumber].rangeLow)
	{
		bitWrite( this->action , actorNumber , 1) ;

	#if DEBUG == 1 

		Serial.println(F("actor ON "));
	
	#endif
				
	}
	//measured value higher than maximum range : deactivate actor
	else if(measurment > this->actors[actorNumber].rangeHigh)
	{
		bitWrite( this->action , actorNumber , 0) ;

	#if DEBUG == 1 

		Serial.println(F("actor OFF "));
	
	#endif
	
	}


}


/**
*	Jetpack::invertedAction(actorNumber , measured value):
*	This method is provided to
*	handle inverted actors.
*	it changes the action according to wether the
*	measured value is:
*	> rangeHigh (activate actor)
*	< rangeLow ( deactivate actor )
*/
void Jetpack::invertedAction(int actorNumber,float measurment)
{
#if DEBUG == 1
	
	Serial.print(F("inverted Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment: "+this->actors[actorNumber].comment);


	Serial.print(F("measured value : "));
	Serial.println(measurment);

	Serial.print(F("high range : "));
	Serial.println(this->actors[actorNumber].rangeHigh);

	Serial.print(F("low range : "));
	Serial.println(this->actors[actorNumber].rangeLow);

#endif

	//measured value lower than minimum range : deactivate actor
	if(measurment < this->actors[actorNumber].rangeLow)
	{
		bitWrite( this->action , actorNumber , 0) ;

	#if DEBUG == 1 

		Serial.println(F("actor OFF "));
	
	#endif

	}
	//measured value higher than maximum range : activate actor
	else if(measurment > this->actors[actorNumber].rangeHigh)
	{
		bitWrite( this->action , actorNumber , 1) ;

	#if DEBUG == 1 

		Serial.println(F("actor ON "));
	
	#endif

	}


}

/**
*	Jetpack::temporalActionOff(actorNumber ):
*	This method is provided to
*	handle temporal actors.
*	it changes the action according to:
*	
*	currentTime - startTime > timeHigh : deactivate actor 
*
*/
void Jetpack::temporalActionOff(int actorNumber)
{

#if DEBUG == 1
	Serial.println(F("temporalActionOff"));
	Serial.print(F("temporal Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment: "+this->actors[actorNumber].comment);


	Serial.print(F("millis : "));
	Serial.println(millis());

	Serial.print(F("actif Time : "));
	Serial.println(this->actors[actorNumber].actifTime);

	Serial.print(F("high time : "));
	Serial.println(this->actors[actorNumber].timeHigh);


#endif
	
	if( ( millis()- this->actors[actorNumber].actifTime  ) >= (  this->actors[actorNumber].timeHigh  ) || this->actors[actorNumber].actifTime == 0)
	{
		//stop the actor
		bitWrite( this->action , actorNumber , 0) ;

		//make the actor inactif:
		this->actors[actorNumber].actif=0;

		//start the low timer
		this->actors[actorNumber].inactifTime=millis();

	#if DEBUG == 1 

		Serial.println(F("actor OFF "));
	
	#endif
				
	}	
}


/**
*	Jetpack::mixedTemporalActionOff(actorNumber, measured value ):
*	This method is provided to
*	handle mixed temporal actors.
*	it changes the action according to:
*	
*	currentTime - startTime >= timeHigh :  
*		measured value >= rangeHigh : deactivate actor
*		measured value < rangeHigh : activate actor
*/
void Jetpack::mixedTemporalActionOff(int actorNumber,float measurment)
{

#if DEBUG == 1
	
	Serial.print("------- mixed Temporal Actor N° : ");
	Serial.println(actorNumber);
	Serial.println("Comment : " + this->actors[actorNumber].comment);

	Serial.print("measured value : ");
	Serial.println(measurment);

	Serial.print("high range : ");
	Serial.println(this->actors[actorNumber].rangeHigh);

	Serial.print("time high : ");
	Serial.println(this->actors[actorNumber].timeHigh);

	Serial.print("actif Time : ");
	Serial.println(this->actors[actorNumber].actifTime);

	Serial.print(F("millis : "));
	Serial.println(millis());

#endif
	if( ( millis()- this->actors[actorNumber].actifTime  ) >= (  this->actors[actorNumber].timeHigh  ) )
	{	
		if( measurment >= this->actors[actorNumber].rangeHigh )
		{
			//stop the actor
			bitWrite( this->action , actorNumber , 0) ;

			//make the actor inactif:
			this->actors[actorNumber].actif=0;

			//start the low timer
			this->actors[actorNumber].inactifTime=millis();

		#if DEBUG == 1 

			Serial.print(F("actor was on for at least "));
			Serial.print(this->actors[actorNumber].timeHigh);
			Serial.println(F(" ms "));

			Serial.print(measurment);
			Serial.print(F(" > " ));
			Serial.println(this->actors[actorNumber].rangeHigh);

			
			Serial.println(F("actor OFF "));

		#endif

		}
		else 
		{
			bitWrite( this->action , actorNumber , 1) ;

		#if DEBUG == 1 
			
			Serial.print(F("actor was on for at least "));
			Serial.print(this->actors[actorNumber].timeHigh);
			Serial.println(F(" ms "));

			Serial.print(measurment);
			Serial.print(F(" < " ));
			Serial.println(this->actors[actorNumber].rangeHigh);

			Serial.println(F("actor ON "));

		#endif				

		}			
	}

}


/**
*	Jetpack::temporalActionOn(actorNumber ):
*	This method is provided to
*	handle temporal actors.
*	it changes the action according to :
*
*	currentTime - stopTime > timeLow : activate actor 
*
*/
void Jetpack::temporalActionOn(int actorNumber)
{

#if DEBUG == 1
	Serial.println(F("temporalActionOn"));
	Serial.print(F("temporal Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment: "+this->actors[actorNumber].comment);


	Serial.print(F("millis : "));
	Serial.println(millis());

	Serial.print(F("inactif Time : "));
	Serial.println(this->actors[actorNumber].inactifTime);

	Serial.print(F("low time : "));
	Serial.println(this->actors[actorNumber].timeLow);


#endif

	 if( ( millis() - this->actors[actorNumber].inactifTime ) >= (  this->actors[actorNumber].timeLow  ) )
	{
		//start the actor
		bitWrite( this->action , actorNumber , 1) ;

		//make the actor actif:
		this->actors[actorNumber].actif=1;

		//start the low timer
		this->actors[actorNumber].actifTime=millis();

	#if DEBUG == 1 

		Serial.println(F("actor ON "));

	#endif				

	}

}

/**
*	Jetpack::mixedTemporalActionOn(actorNumber, measured value ):
*	This method is provided to
*	handle mixed temporal actors.
*	it changes the action according to :
*
*	currentTime - stopTime > timeLow :   
*		measured value >= rangeLow : deactivate actor
*		measured value < rangeLow : activate actor
*
*/
void Jetpack::mixedTemporalActionOn(int actorNumber,float measurment)
{

#if DEBUG == 1
	
	Serial.print(F("------- mixed Temporal Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment : " + this->actors[actorNumber].comment);

	Serial.print(F("measured value : "));
	Serial.println(measurment);

	Serial.print(F("low range : "));
	Serial.println(this->actors[actorNumber].rangeLow);

	Serial.print(F("time low : "));
	Serial.println(this->actors[actorNumber].timeLow);

	Serial.print(F("inactif Time : "));
	Serial.println(this->actors[actorNumber].inactifTime);

	Serial.print(F("millis : "));
	Serial.println(millis());

#endif

	if( ( millis() - this->actors[actorNumber].inactifTime ) >= (  this->actors[actorNumber].timeLow  ) )
	{
		if( measurment < this->actors[actorNumber].rangeLow )
		{
			//start the actor
			bitWrite( this->action , actorNumber , 1) ;

			//make the actor actif:
			this->actors[actorNumber].actif=1;

			//start the low timer
			this->actors[actorNumber].actifTime=millis();

		#if DEBUG == 1 

			Serial.print(F("actor was off for at least "));
			Serial.print(this->actors[actorNumber].timeLow);
			Serial.println(F(" ms "));

			Serial.print(measurment);
			Serial.print(F(" < " ));
			Serial.println(this->actors[actorNumber].rangeLow);
	
			Serial.println(F("actor ON "));
	
		#endif	

		}
		else 
		{
			bitWrite( this->action , actorNumber , 0) ;	

		#if DEBUG == 1 

			Serial.print(F("actor was off for at least "));
			Serial.print(this->actors[actorNumber].timeLow);
			Serial.println(F(" ms "));

			Serial.print(measurment);
			Serial.print(F(" > " ));
			Serial.println(this->actors[actorNumber].rangeLow);

			Serial.println(F("actor OFF "));
	
		#endif				

		}

	}

	
}


/**
*	Jetpack::hourAction(actorNumber, current hour ):
*	This method is provided to
*	handle hour actors.
*	it changes the action according to:
*	
*	hour >= hourLow : deactivate the actor
*	hour >= hourHigh : activate the actor 
*
*/
void Jetpack::hourAction(int actorNumber, int hour)
{

#if DEBUG == 1
	
	Serial.print(F("------- hour Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment : " + this->actors[actorNumber].comment);

	Serial.print(F(" hour : "));
	Serial.println(hour);

	Serial.print(F("high hour : "));
	Serial.println(this->actors[actorNumber].hourHigh);

	Serial.print(F("low hour : "));
	Serial.println(this->actors[actorNumber].hourLow);

	Serial.print(F("inverted Flag : "));
	Serial.println(this->actors[actorNumber].inverted);
	Serial.println();
#endif

	if (this->actors[actorNumber].hourHigh < this->actors[actorNumber].hourLow)
	{
		//stop the actor	
		if(hour >= this->actors[actorNumber].hourLow || hour < this->actors[actorNumber].hourHigh)
		{
			if (this->actors[actorNumber].inverted)
			{
				bitWrite( this->action , actorNumber , 1) ;
			}
			else 
			{
				bitWrite( this->action , actorNumber , 0) ;
			}

		#if DEBUG == 1 

			Serial.println(F("Daymode"));
			Serial.println(F("actor OFF "));

		#endif	

		}
		//starting the actor
		else
		{
			if (this->actors[actorNumber].inverted)
			{
				bitWrite( this->action , actorNumber , 0) ;
			}
			else 
			{
				bitWrite( this->action , actorNumber , 1) ;
			}

		#if DEBUG == 1 

			Serial.println(F("DayMode"));
			Serial.println(F("actor ON "));

		#endif	
		
		}
	}
	else
	{
		//stop the actor in Nght mode ie a light that is on over night
		if(hour >= this->actors[actorNumber].hourLow && hour < this->actors[actorNumber].hourHigh)
		{
			if (this->actors[actorNumber].inverted)
			{
				bitWrite( this->action , actorNumber , 1) ;
			}
			else 
			{
				bitWrite( this->action , actorNumber , 0) ;
			}

		#if DEBUG == 1 

			Serial.println(F("Nightmode"));
			Serial.println(F("Actor OFF "));

		#endif	

		}
		//starting the actor
		else
		{
			if (this->actors[actorNumber].inverted)
			{
				bitWrite( this->action , actorNumber , 0) ;
			}
			else 
			{
				bitWrite( this->action , actorNumber , 1) ;
			}

		#if DEBUG == 1 

			Serial.println(F("Hightmode"));
			Serial.println(F("Actor ON "));

		#endif	
		
		}
	}
}


/**
*	Jetpack::mixedHourAction(actorNumber, current hour, measured value ):
*	This method is provided to
*	handle mixed hour actors.
*	it changes the action according to :
*
*	hour >= hourLow :
*		-measuredValue >= rangeHigh : deactivate actor
*		-measured < rangeHigh : activate actor
*
*	hour >= hourHigh :
*		-measuredValue < rangeLow : activate actor
*		-measuredValue >=rangeLow : activate actor
*/
void Jetpack::mixedHourAction(int actorNumber,int hour, float measurment)
{

#if DEBUG == 1
	
	Serial.print(F("------- mixed hour Actor N° : "));
	Serial.println(actorNumber);
	Serial.print("Comment : " + this->actors[actorNumber].comment);

	Serial.print(F(" hour : "));
	Serial.println(hour);

	Serial.print(F("high hour : "));
	Serial.println(this->actors[actorNumber].hourHigh);

	Serial.print(F("low hour : "));
	Serial.println(this->actors[actorNumber].hourLow);

	Serial.print(F("measured value : "));
	Serial.println(measurment);

	Serial.print(F("high range : "));
	Serial.println(this->actors[actorNumber].rangeHigh);

	Serial.print(F("low range : "));
	Serial.println(this->actors[actorNumber].rangeLow);

#endif



	if (measurment <= this->actors[actorNumber].rangeLow && this->actors[actorNumber].failsave == true)
	{
		this->actors[actorNumber].failsave = false;
		Serial.println("!!! Reset Failsave !!!");
	}

	else if (measurment >= this->actors[actorNumber].rangeHigh && this->actors[actorNumber].failsave == false)
	{
		this->actors[actorNumber].failsave = true;
		Serial.println("!!! Engage Failsave !!!");
	}


	if (this->actors[actorNumber].hourHigh < this->actors[actorNumber].hourLow)
	{
		//stop the actor	
		if((hour >= this->actors[actorNumber].hourLow || hour < this->actors[actorNumber].hourHigh) || this->actors[actorNumber].failsave == true)
		{
			if (this->actors[actorNumber].inverted)
			{
				bitWrite( this->action , actorNumber , 1) ;
			}
			else 
			{
				bitWrite( this->action , actorNumber , 0) ;
			}

		#if DEBUG == 1 

			Serial.println(F("Daymode"));
			Serial.println(F("Onboard Actor OFF "));
			Serial.print(F("failsave Flag : "));
			Serial.println(this->actors[actorNumber].failsave);

		#endif	

		}
		//starting the actor
		else if (this->actors[actorNumber].failsave == false)
		{
			if (this->actors[actorNumber].inverted)
			{
				bitWrite( this->action , actorNumber , 0) ;
			}
			else 
			{
				bitWrite( this->action , actorNumber , 1) ;
			}

		#if DEBUG == 1 

			Serial.println(F("DayMode"));
			Serial.println(F("Onboard Actor ON "));

		#endif	
		
		}
	}
	else
	{
		//stop the actor in Nght mode ie a light that is on over night
		if((hour >= this->actors[actorNumber].hourLow && hour < this->actors[actorNumber].hourHigh) || this->actors[actorNumber].failsave == true )
		{
			if (this->actors[actorNumber].inverted)
			{
				bitWrite( this->action , actorNumber , 1) ;
			}
			else 
			{
				bitWrite( this->action , actorNumber , 0) ;
			}

		#if DEBUG == 1 

			Serial.println(F("Nightmode"));
			Serial.println(F("Onboard Actor OFF "));

		#endif	

		}
		//starting the actor
		else if (this->actors[actorNumber].failsave == false)
		{
			if (this->actors[actorNumber].inverted)
			{
				bitWrite( this->action , actorNumber , 0) ;
			}
			else 
			{
				bitWrite( this->action , actorNumber , 1) ;
			}

		#if DEBUG == 1 

			Serial.println(F("Nightmode"));
			Serial.println(F("Onboard Actor ON "));

		#endif	
		
		}
	}
}

/*********************************
*	Dedicated method for Spirulina
*	Jetpack::perilstaticSpiru(int actorNumber, int hour): 
*	Pump will be activated to feed one quantity (mL) during period 
*		Day :  	ON during rangeHigh (ms) / Off timeHigh (ms)
*		Night : ON during rangeLow (ms) / Off timeLow (ms)
* 	Exemple jetPackConfig.json:
		"Act4":
			{
				"actif":0,
				"inverted":0,
				"temporal":0,
				"low":[8,0,11,0],
				"high":[100,2391,8,0],
				"type":["","perilstaticSpiru"],
				"comment":"Perilstatic Pump Spiru Actor"
			},
*		(Day Mode) 		if 8 >= Time < 11 
*		--> ON Timer = (100/8)/(2391/1000)
*		--> OFF Timer = 8-11*3600*1000 - (100+8)/ (2391/1000) 
*		(Night Mode) 	if 17 =< Time < 8 
*		--> ON during 500 000 ms / OFF during 1 200 000 ms
*/	
void Jetpack::perilstaticSpiru(int actorNumber, int hour)
{
	bool toActive = false;
	
	/*
	long debit = actors[actorNumber].timeHigh/10000; 		//Debit of Specific Pump in mL/sec
	long circuitCapacity = actors[actorNumber].rangeLow;	//Size of circuit network (mL)
	int timePeriodFeed = actors[actorNumber].hourLow - actors[actorNumber].hourHigh*3600;
	long toFeed = actors[actorNumber].rangeHigh + circuitCapacity;
	long feedPeriod = circuitCapacity;						//to Feed drop peer drop (mL)
	
	unsigned long timerOn = feedPeriod / debit *1000; 			// ( msec )
	unsigned long timerOff = ((timePeriodFeed - (toFeed + circuitCapacity))/(toFeed/feedPeriod))*1000 ;
	*/

	long timerOn = 33600;
	long timerOff = 539712;

#if DEBUG == 1
	Serial.println("************************************");
	Serial.print(F("- Spiru Perilstatic Pump Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment : " + this->actors[actorNumber].comment);
	Serial.print(F("- hour : "));
	Serial.println(hour);
	Serial.print(F("- millis : "));
	Serial.println(millis());
	Serial.print(F("- high hour : "));
	Serial.println(this->actors[actorNumber].hourHigh);
	Serial.print(F("- low hour : "));
	Serial.println(this->actors[actorNumber].hourLow);
	/*
	Serial.println("- debit (mL/sec) 	: " + debit);
	Serial.println("- toFeed (mL) 		: " + toFeed);
	Serial.println("- feed period (mL) 	: " + feedPeriod);
	Serial.println("- Circuit Capacity 	: " + circuitCapacity);*/

	Serial.print("- timerOn (msec)	: " );
	Serial.println(timerOn);
	Serial.print("- timerOff (msec)	: ");
	Serial.println(timerOff);
#endif
	
	//Day / Night Mode config
	if((hour < this->actors[actorNumber].hourLow && hour >= this->actors[actorNumber].hourHigh) )
	{
		#if DEBUG == 1
			Serial.println("- Day mode");
		#endif	
		// On / Off Balanced 
		if (this->actors[actorNumber].actif==1) {
			#if DEBUG == 1
				Serial.print("- Actif=1 / diff : ");
				Serial.println(millis() - this->actors[actorNumber].actifTime);
			#endif

			if ( (millis() - this->actors[actorNumber].actifTime) > timerOn ) 
			{
				//Stop the actor... during TimerOff... 
				toActive = false;
				this->actors[actorNumber].inactifTime = millis();
			}
			else {
				toActive = true;
			}
		}
		else {
			#if DEBUG == 1
				Serial.print("- Actif=0 / diff : ");
				Serial.println(millis() - this->actors[actorNumber].inactifTime);
			#endif

			if ( (millis() - this->actors[actorNumber].inactifTime) > timerOff ) 
			{
				toActive = true;
				this->actors[actorNumber].actifTime = millis();
			}
			else {
				toActive = false;
			}
		} 
	} else {
		#if DEBUG == 1
			Serial.println("- Night mode");
		#endif
			
		toActive = false;
	}
	
	// Set the Actor
	if (toActive){
		if (this->actors[actorNumber].inverted) {
			bitWrite( this->action , actorNumber , 0) ;
		} else {
			bitWrite( this->action , actorNumber , 1) ;
		}
		this->actors[actorNumber].actif=1;
		#if DEBUG == 1 
			Serial.println(F("------> Actor ON "));
		#endif
	} else {
		if (this->actors[actorNumber].inverted) {
			bitWrite( this->action , actorNumber , 1) ;
		} else {
			bitWrite( this->action , actorNumber , 0) ;
		}
		
		this->actors[actorNumber].actif=0;
		#if DEBUG == 1 
			Serial.println(F("------> Actor OFF "));
		#endif
	}
	Serial.println("************************");	
}
/*********************************
*	Dedicated method for Spirulina
*	Jetpack::bublerSpiru(int actorNumber, int hour): 
*	Bubler control function will have to alternate On/Off period regarding Day & Night
*		Day :  	ON during rangeHigh (ms) / Off timeHigh (ms)
*		Night : ON during rangeLow (ms) / Off timeLow (ms)
* 	Exemple jetPackConfig.json:
*		"Act4":
			{
				"actif":0,
				"inverted":0,
				"temporal":0,
				"low":[500000,1200000,17,0],
				"high":[2700000,600000,8,0],
				"type":["","bublerSpiru"]
			}
*		(Day Mode) 		if 8 >= Time < 17 
*		--> ON during 2 700 000 ms / OFF During 600 000 ms 
*		(Night Mode) 	if 17 =< Time < 8 
*		--> ON during 500 000 ms / OFF during 1 200 000 ms
*/	
void Jetpack::bublerSpiru(int actorNumber, int hour)
{

#if DEBUG == 1
	Serial.println("**************************");
	Serial.print(F("- Spiru Bubler Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment : " + this->actors[actorNumber].comment);
	Serial.print(F("- hour : "));
	Serial.println(hour);
	Serial.print(F("- millis : "));
	Serial.println(millis());
	Serial.print(F("- high hour : "));
	Serial.println(this->actors[actorNumber].hourHigh);
	Serial.print(F("- low hour : "));
	Serial.println(this->actors[actorNumber].hourLow);
	Serial.print(F("- high range On/Off (sec) : "));
	Serial.print(this->actors[actorNumber].rangeHigh);
	Serial.print(" / ");
	Serial.println(this->actors[actorNumber].timeHigh);
	Serial.print(F("- low range On/Off (sec) : "));
	Serial.print(this->actors[actorNumber].rangeLow);
	Serial.print(" / ");
	Serial.println(this->actors[actorNumber].timeLow);
	Serial.print(F("- actif Time (ms): "));
	Serial.println(this->actors[actorNumber].actifTime);
	Serial.print(F("- inactif Time (ms): "));
	Serial.println(this->actors[actorNumber].inactifTime);
#endif
	bool toActive = false;
	unsigned long timerOn = 0;
	unsigned long timerOff = 0;

	//Day / Night Mode config
	if((hour < this->actors[actorNumber].hourLow && hour >= this->actors[actorNumber].hourHigh) )
	{
		Serial.println("- Day mode");
		timerOn = this->actors[actorNumber].rangeHigh;
		timerOff = this->actors[actorNumber].timeHigh;
	} else {
		Serial.println("- Night mode");
		timerOn = this->actors[actorNumber].rangeLow;
		timerOff = this->actors[actorNumber].timeLow;
	}
	
	Serial.print("- timerOn / timerOff : ");
	Serial.print(timerOn);
	Serial.print(" / ");
	Serial.println(timerOff);

	long diff = 0;

	// On / Off Balanced 
	if (this->actors[actorNumber].actif==1) {
		#if DEBUG == 1
			Serial.print("- Actif=1 / diff : ");
			Serial.println(millis() - this->actors[actorNumber].actifTime);
		#endif

		if ( (millis() - this->actors[actorNumber].actifTime) > timerOn ) 
		{
			toActive = false;
			this->actors[actorNumber].inactifTime = millis();
		}
		else {
			toActive = true;
		}
	}
	else {
		#if DEBUG == 1
			Serial.print("- Actif=0 / diff : ");
			Serial.println(millis() - this->actors[actorNumber].inactifTime);
		#endif

		if ( (millis() - this->actors[actorNumber].inactifTime) > timerOff ) 
		{
			toActive = true;
			this->actors[actorNumber].actifTime = millis();
		}
		else {
			toActive = false;
		}
	}

	// Set the Actor
	if (toActive){
		
		if (this->actors[actorNumber].inverted) {
			bitWrite( this->action , actorNumber , 0) ;
		} else {
			bitWrite( this->action , actorNumber , 1) ;
		}
		
		this->actors[actorNumber].actif=1;
		#if DEBUG == 1 
			Serial.println(F("------> Actor ON "));
		#endif
	} else {
		if (this->actors[actorNumber].inverted) {
			bitWrite( this->action , actorNumber , 1) ;
		} else {
			bitWrite( this->action , actorNumber , 0) ;
		}
		
		this->actors[actorNumber].actif=0;
		#if DEBUG == 1 
			Serial.println(F("------> Actor OFF "));
		#endif
	}
	Serial.println("************************");	
}

/*********************************
*	Dedicated method for Spirulina
*	Spiru Temperature Actor
*	Jetpack::heatSpiru(int actorNumber,int hour, float measurment)
*	ON when :  
*		measurment < rangeLow
*		Day mode ( hour >= hourHigh && hour < hourLow ) On until measurment <= rangeHigh
*	OFF when : 
*		measurment > rangeHigh 
*		Night mode ( hour >= hourHigh || hour < hourLow )
*	Hysteresis : Using timeHigh & timeLow (ms) with
*		Set On if inactifHysteresis > timeLow
*		Set Off if actifHysteresis > timeHigh
*
* 	Exemple - jetPackConfig.json :
*	----------------------------
		"Act3":
			{
				"actif":0,
				"inverted":0,
				"temporal":0,
				"low":[15,100000,17,0],
				"high":[22,300000,8,0],
				"type":["TempMC","heatSpiru"]
			},
*		(Day Mode) if 8 <= Time < 17  
*			--> ON until 22°C.  
*		(Night Mode)  if Time >= 17 || Time < 8
*			--> OFF
*		Temperature Range Control 
*			--> ON if measurement < 15°C
*			--> OFF if measurement > 22°C
*		(Hysteresis
*			--> ON during MAX : 300000 ms 
*			--> OFF during : 100000 ms
*/
void Jetpack::heatSpiru(int actorNumber,int hour, float measurment)
{

#if DEBUG == 1
	Serial.println("************************");
	Serial.print(F("- Spiru Heat Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment : " + this->actors[actorNumber].comment);

	Serial.print(F("- hour : "));
	Serial.println(hour);
	Serial.print(F("- high hour : "));
	Serial.println(this->actors[actorNumber].hourHigh);
	Serial.print(F("- low hour : "));
	Serial.println(this->actors[actorNumber].hourLow);
	Serial.print(F("- measured value : "));
	Serial.println(measurment);
	Serial.print(F("- high range : "));
	Serial.println(this->actors[actorNumber].rangeHigh);
	Serial.print(F("- low range : "));
	Serial.println(this->actors[actorNumber].rangeLow);

#endif
	bool toActive = false;
	
	if(hour < this->actors[actorNumber].hourLow && hour >= this->actors[actorNumber].hourHigh )
	{
		Serial.println("- Day mode...");
		toActive = true;
		this->actors[actorNumber].actifTime = millis();
	} else {
		Serial.println("- Night mode");
		toActive = false;
		this->actors[actorNumber].inactifTime=millis();
	}

	// In any case... Active if down the LowLimit !
	if (measurment <= this->actors[actorNumber].rangeLow )
	{
		Serial.println("!!! Range Low... HEAT ... !!!");
		toActive = true;
		this->actors[actorNumber].actifTime = millis();
	}

	// In any case... Shutdown if up to high !
	if (measurment >= this->actors[actorNumber].rangeHigh) 
	{
		Serial.println("!!! Too High !!!");
		toActive = false;
		this->actors[actorNumber].inactifTime=millis();
	}


	//Hysteresis Managment 
	//hysteresisOn : Max Period ON ( ms ) - high:["","6000",...]
	//hysteresisOff : Max Period OFF ( ms ) - low:["","1000",...]
	bool hysteresis = true;
	unsigned long hysteresisOn = this->actors[actorNumber].timeHigh;
	unsigned long hysteresisOff = this->actors[actorNumber].timeLow;

	if (this->actors[actorNumber].actif==1) {
		#if DEBUG == 1
			Serial.println("- Hysteresis check ON ");
			Serial.print("-- diff = ");
			Serial.println(millis() - this->actors[actorNumber].actifHysteresis);
			Serial.print("-- hysteresisOn=");
			Serial.println(hysteresisOn);
		#endif

		if( millis() - this->actors[actorNumber].actifHysteresis > hysteresisOn ) {
			#if DEBUG == 1
				Serial.println("Hysteresis long... shutoff for a while...");
			#endif

			hysteresis = false;
			this->actors[actorNumber].inactifHysteresis=millis();
			}
		}
	else {
		hysteresis = false;
		#if DEBUG == 1
			Serial.println("- Hysteresis check OFF");
			Serial.print("-- diff = ");
			Serial.println(millis() - this->actors[actorNumber].inactifHysteresis);
			Serial.print("-- hysteresisOff=");
			Serial.println(hysteresisOff);
		#endif

		if( millis() - this->actors[actorNumber].inactifHysteresis > hysteresisOff ) {
				#if DEBUG == 1
					Serial.println("Hysteresis Ok... can go up");
				#endif

				hysteresis = true;
				this->actors[actorNumber].actifHysteresis=millis();
		} 
	}

	//Decision :
	if (toActive && hysteresis)
	{
		#if DEBUG == 1
			Serial.println(F("------> Actor ON "));
		#endif

		bitWrite( this->action , actorNumber , 1) ;
		this->actors[actorNumber].actif=1;
	}
	else
	{
		Serial.println(F("------> Actor OFF "));
		bitWrite( this->action , actorNumber , 0) ;
		this->actors[actorNumber].actif=0;
	}
}

/**
*	Jetpack::minteAction(actorNumber, current minute ):
*	This method is provided to
*	handle minute actors.
*	it changes the action according to:
*	
*	minute >= minuteLow : deactivate the actor
*	minute >= minuteHigh : activate the actor 
*
*/
void Jetpack::minuteAction(int actorNumber,int minute)
{

#if DEBUG == 1
	
	Serial.print(F("------- minute Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment: "+this->actors[actorNumber].comment);

	Serial.print(F(" minute : "));
	Serial.println(minute);

	Serial.print(F("high minute : "));
	Serial.println(this->actors[actorNumber].minuteHigh);

	Serial.print(F("low minute : "));
	Serial.println(this->actors[actorNumber].minuteLow);

#endif

	//stop the actor	
	if(minute <= this->actors[actorNumber].minuteLow)
	{
		bitWrite( this->action , actorNumber , 0) ;

	#if DEBUG == 1 

		Serial.println(F("actor OFF "));

	#endif	

	}	
	//starting the actor
	else if(minute >= this->actors[actorNumber].minuteHigh)
	{
		bitWrite( this->action , actorNumber , 1) ;

	#if DEBUG == 1 

		Serial.println(F("actor ON "));

	#endif	

	}

} 

/**
*	Jetpack::mixedMinuteAction(actorNumber, current minute, measured value ):
*	This method is provided to
*	handle mixed minute actors.
*	it changes the action according to :
*
*	minute >= minuteLow :
*		-measuredValue >= rangeHigh : deactivate actor
*		-measured < rangeHigh : activate actor
*
*	minute >= minuteHigh :
*		-measuredValue < rangeLow : activate actor
*		-measuredValue >=rangeLow : activate actor
*/
void Jetpack::mixedMinuteAction(int actorNumber,int minute,float measurment)
{

#if DEBUG == 1
	
	Serial.print(F("------- mixed minute Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment : " + this->actors[actorNumber].comment);

	Serial.print(F(" minute : "));
	Serial.println(minute);

	Serial.print(F("high minute : "));
	Serial.println(this->actors[actorNumber].minuteHigh);

	Serial.print(F("low minute : "));
	Serial.println(this->actors[actorNumber].minuteLow);

	Serial.print(F("measured value : "));
	Serial.println(measurment);

	Serial.print(F("high range : "));
	Serial.println(this->actors[actorNumber].rangeHigh);

	Serial.print(F("low range : "));
	Serial.println(this->actors[actorNumber].rangeLow);

#endif
	//stop the actor	
	if(minute <= this->actors[actorNumber].minuteLow)
	{
			if( measurment > this->actors[actorNumber].rangeHigh )
			{
				bitWrite( this->action , actorNumber , 0) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" > " ));
				Serial.println(this->actors[actorNumber].rangeHigh);

				Serial.println(F("actor OFF "));

			#endif
	
			}
			else 
			{
				bitWrite( this->action , actorNumber , 1) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" < " ));
				Serial.println(this->actors[actorNumber].rangeHigh);

				Serial.println(F("actor ON "));

			#endif	
				
			}
	}	
	//starting the actor
	else if(minute >= this->actors[actorNumber].minuteHigh)
	{
		if( measurment < this->actors[actorNumber].rangeLow )
		{
			bitWrite( this->action , actorNumber , 1) ;

		#if DEBUG == 1 

			Serial.print(measurment);
			Serial.print(F(" < " ));
			Serial.println(this->actors[actorNumber].rangeLow);

			Serial.println(F("actor ON "));

		#endif	

		}
		else 
		{
			bitWrite( this->action , actorNumber , 0) ;
		
		#if DEBUG == 1 

			Serial.print(measurment);
			Serial.print(F(" > " ));
			Serial.println(this->actors[actorNumber].rangeLow);

			Serial.println(F("actor OFF "));

		#endif	
			
		}

	}

}


/**
*	Jetpack::minteAction(actorNumber, current hour,current minute ):
*	This method is provided to
*	handle hour minute actors.
*	it changes the action according to:
*	
*	hour == hourLow : 
*		minute >= minuteLow : deactivate the actor
*
*	hour >  hourLow : deactivate the actor
*
*	hour == hourHigh : 
*		minute >= minteHigh : activate the actor 
*
*	hour >  hourHigh : activate the actor
*/
void Jetpack::hourMinuteAction(int actorNumber,int hour,int minute)
{

#if DEBUG == 1

	Serial.print(F("------- hourMinute Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment : " + this->actors[actorNumber].comment);

	Serial.print(F(" hour : "));
	Serial.println(hour);
	Serial.print(F(" minute : "));
	Serial.println(minute);

	Serial.print(F("high hour : "));
	Serial.println(this->actors[actorNumber].hourHigh);

	Serial.print(F("high minute : "));
	Serial.println(this->actors[actorNumber].minuteHigh);

	Serial.print(F("low hour : "));
	Serial.println(this->actors[actorNumber].hourLow);

	Serial.print(F("low minute : "));
	Serial.println(this->actors[actorNumber].minuteLow);

#endif
	//stop the actor
	if(hour==this->actors[actorNumber].hourLow)
	{
		if(minute>= this->actors[actorNumber].minuteLow)
		{
			bitWrite( this->action , actorNumber , 0) ;
		#if DEBUG == 1 

			Serial.println(F("actor OFF "));

		#endif	
		}
	}
	else if(hour > this->actors[actorNumber].hourLow)
	{

		bitWrite( this->action , actorNumber , 0) ;
	#if DEBUG == 1 

		Serial.println(F("actor OFF "));

	#endif	
	
	}
	//start the actor
	else if(hour==this->actors[actorNumber].hourHigh)
	{
		if(minute>= this->actors[actorNumber].minuteHigh)
		{
			bitWrite( this->action , actorNumber , 1) ;

		#if DEBUG == 1 

			Serial.println(F("actor ON "));

		#endif	
		}
	}
	else if(hour > this->actors[actorNumber].hourHigh)
	{

		bitWrite( this->action , actorNumber , 1) ;

	#if DEBUG == 1 

		Serial.println(F("actor ON "));

	#endif		

	}

	
}

/**
*	Jetpack::minteAction(actorNumber, current hour,current minute , measured Value ):
*	This method is provided to
*	handle hour minute actors.
*	it changes the action according to:
*	
*	hour == hourLow : 
*		minute >= minuteLow : 
*			measuredValue >= rangeHigh : deactivate actor
*			measuredValue < rangeHigh : activate actor
*	
*	hour >  hourLow : 
*		measuredValue >= rangeHigh : deactivate actor
*		measuredValue < rangeHigh : activate actor
*
*	hour == hourHigh : 
*		minute >= minteHigh : 
*			measuredValue >= rangeLow : deactivate actor
*			measuredValue < rangeLow : activate actor 
*
*	hour >  hourHigh :
*		measuredValue >= rangeLow : deactivate actor
*		measuredValue < rangeLow : activate actor 
*
*/
void Jetpack::mixedHourMinuteAction(int actorNumber,int hour,int minute ,float measurment)
{

#if DEBUG == 1
	
	Serial.print(F("------- hourMinute Actor N° : "));
	Serial.println(actorNumber);
	Serial.println("Comment : " + this->actors[actorNumber].comment);

	Serial.print(F(" hour : "));
	Serial.println(hour);
	Serial.print(F(" minute : "));
	Serial.println(minute);

	Serial.print(F("high hour : "));
	Serial.println(this->actors[actorNumber].hourHigh);

	Serial.print(F("high minute : "));
	Serial.println(this->actors[actorNumber].minuteHigh);

	Serial.print(F("low hour : "));
	Serial.println(this->actors[actorNumber].hourLow);

	Serial.print(F("low minute : "));
	Serial.println(this->actors[actorNumber].minuteLow);

	Serial.print(F("measured value : "));
	Serial.println(measurment);

	Serial.print(F("high range : "));
	Serial.println(this->actors[actorNumber].rangeHigh);

	Serial.print(F("low range : "));
	Serial.println(this->actors[actorNumber].rangeLow);

#endif
	//stop the actor
	if(hour==this->actors[actorNumber].hourLow)
	{
		if(minute>= this->actors[actorNumber].minuteLow)
		{
			if( measurment >= this->actors[actorNumber].rangeHigh )
			{
				bitWrite( this->action , actorNumber , 0) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" >= " ));
				Serial.println(this->actors[actorNumber].rangeHigh);

				Serial.println(F("actor OFF "));

			#endif	

			}
			else 
			{
				bitWrite( this->action , actorNumber , 1) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" < " ));
				Serial.println(this->actors[actorNumber].rangeHigh);

				Serial.println(F("actor ON "));

			#endif	
				
			}
		}
	}
	else if(hour > this->actors[actorNumber].hourLow)
	{

		if( measurment >= this->actors[actorNumber].rangeHigh )
		{
			bitWrite( this->action , actorNumber , 0) ;

		#if DEBUG == 1 

			Serial.print(measurment);
			Serial.print(F(" >= " ));
			Serial.println(this->actors[actorNumber].rangeHigh);

			Serial.println(F("actor OFF "));

		#endif	

		}
		else 
		{
			bitWrite( this->action , actorNumber , 1) ;

		#if DEBUG == 1 

			Serial.print(measurment);
			Serial.print(F(" < " ));
			Serial.println(this->actors[actorNumber].rangeHigh);

			Serial.println(F("actor ON "));

		#endif	
			
		}


	}
	//start the actor
	else if(hour==this->actors[actorNumber].hourHigh)
	{
		if(minute>= this->actors[actorNumber].minuteHigh)
		{
			if( measurment < this->actors[actorNumber].rangeLow )
			{
				bitWrite( this->action , actorNumber , 1) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" < " ));
				Serial.println(this->actors[actorNumber].rangeLow);

				Serial.println(F("actor ON "));

			#endif	

			}
			else 
			{
				bitWrite( this->action , actorNumber , 0) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.println(F(" > " ));
				Serial.print(this->actors[actorNumber].rangeLow);

				Serial.println(F("actor OFF "));

			#endif	
				
			}
		}
	}
	else if(hour > this->actors[actorNumber].hourHigh)
	{

		if( measurment < this->actors[actorNumber].rangeLow )
		{
			bitWrite( this->action , actorNumber , 1) ;

		#if DEBUG == 1 

			Serial.print(measurment);
			Serial.print(F(" < " ));
			Serial.println(this->actors[actorNumber].rangeLow);

			Serial.println(F("actor ON "));

		#endif	

		}
		else 
		{
			bitWrite( this->action , actorNumber , 0) ;

		#if DEBUG == 1 

			Serial.print(measurment);
			Serial.println(F(" > " ));
			Serial.print(this->actors[actorNumber].rangeLow);

			Serial.println(F("actor OFF "));

		#endif	
			
		}
	
	}

}


