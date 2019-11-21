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

#include <NeoPixelBus.h>
#include "CoolBoardLed.h"
#include "ArduinoJson.h"

#define DEBUG 0



/**
*	CoolBoardLed::fade ( Red , Green , Blue, Time in seconds ):
*	fade animation:	Fade In over T(seconds)
*			Fade Out over T(seconds)
*/
void CoolBoardLed::fade(int R, int G, int B, float T) 
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoardLed.fade()") );
	Serial.println();
	Serial.print( F("R : ") );
	Serial.println(R);
	Serial.print( F("G : ") );
	Serial.println(G);
	Serial.print( F("B : ") );
	Serial.println(B);
	Serial.print( F("Time : ") );
	Serial.println(T);
	Serial.println();

#endif	
	if(this->ledActive == 1 )
	{
		for (int k = 0; k < 1000; k++) 
		{
			neoPixelLed->SetPixelColor(0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
			neoPixelLed->Show();
			delay(T);
		}
		
		for (int k = 1000; k >= 0; k--) 
		{
			neoPixelLed->SetPixelColor(0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
			neoPixelLed->Show();
			delay(T);
		}
	}
}

/**
*	CoolBoardLed::blink( Red , Green , Blue , Time in seconds ):
*	Blink animation:	Led On for T seconds
				Led off
*/
void CoolBoardLed::blink(int R, int G, int B, float T) 
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoardLed.blink()"));
	Serial.println();
	Serial.print( F("R : "));
	Serial.println(R);
	Serial.print( F("G : ") );
	Serial.println(G);
	Serial.print( F("B : ") );
	Serial.println(B);
	Serial.print( F("Time :") );
	Serial.println(T);
	Serial.println();

#endif	

	if(this->ledActive == 1 )
	{
		neoPixelLed->SetPixelColor(0, RgbColor(R, G, B));
		neoPixelLed->Show();
		delay(T*1000);
		neoPixelLed->SetPixelColor(0, RgbColor(0, 0, 0));
		neoPixelLed->Show();
	}
}

/**
*	CoolBoardLed::fadeIn(Red , Green , Blue , Time in seconds)
*	Fade In animation:	gradual increase over T(seconds)
*/
void CoolBoardLed::fadeIn(int R, int G, int B, float T) 
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoardLed.fadeIn()") );
	Serial.println();
	Serial.print( F("R : ") );
	Serial.println(R);
	Serial.print( F("G : ") );
	Serial.println(G);
	Serial.print( F("B : ") );
	Serial.println(B);
	Serial.print( F("Time :") );
	Serial.println(T);
	Serial.println();

#endif	
	
	if(this->ledActive == 1 )
	{
		for (int k = 0; k < 1000; k++) 
		{
			neoPixelLed->SetPixelColor(0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
			neoPixelLed->Show();
			delay(T);
		}
	}

}

/**
*	CoolBoardLed::fadeOut( Red , Green , Blue , Time in seconds)
*	Fade Out animation:	gradual decrease over T(seconds)
*/
void CoolBoardLed::fadeOut(int R, int G, int B, float T) 
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolBoardLed.fadeOut()" ) );
	Serial.println();
	Serial.print( F("R : ") );
	Serial.println(R);
	Serial.print( F("G : ") );
	Serial.println(G);
	Serial.print( F("B : ") );
	Serial.println(B);
	Serial.print( F("Time :") );
	Serial.println(T);
	Serial.println();

#endif	

	if(this->ledActive == 1 )
	{
		for (int k = 1000; k >= 0; k--) 
		{
			neoPixelLed->SetPixelColor(0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
			neoPixelLed->Show();
			delay(T);
		}
	}
}

/**
*	CoolBoardLed::strobe(Red , Green , Blue , Time in seconds)
*	Strobe animation:	blinks over T(seconds)	
*/
void CoolBoardLed::strobe(int R, int G, int B, float T) 
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoardLed.strobe()") );
	Serial.println();
	Serial.print( F("R : ") );
	Serial.println(R);
	Serial.print( F("G: ") );
	Serial.println(G);
	Serial.print( F("B : ") );
	Serial.println(B);
	Serial.print( F("Time :") );
	Serial.println(T);
	Serial.println();

#endif	

	if(this->ledActive == 1 )
	{	
		for (int k = 1000; k >= 0; k--) 
		{
			neoPixelLed->SetPixelColor(0, RgbColor(R, G, B));
			neoPixelLed->Show();
			delay(T);
			neoPixelLed->SetPixelColor(0, RgbColor(0, 0, 0));
			neoPixelLed->Show();
			delay(T);
		}
	}
}

/**
* 	CoolBoardLed::end() :
*	this method is provided to delete the dynamically created neoPixelLed
*/
void CoolBoardLed::end()
{

#if DEBUG == 1 
	
	Serial.println( F("Entering CoolBoardLed.end()") );

#endif

	delete neoPixelLed;
}


/**
*	CoolBoardLed::begin():
*	This method is provided to start the Led Object 
*	by setting the correct pin and creating a dynamic
*	neoPixelBus  
*/
void CoolBoardLed::begin( )
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoardLed.begin() ") );

#endif
	yield();
	if(this->ledActive == 1 )
	{
		pinMode(5,OUTPUT);
		digitalWrite(5,HIGH);
		neoPixelLed = new NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>(1,2); 
		neoPixelLed->Begin();
		neoPixelLed->Show();

	}

} 

/**
*	CoolBoardLed::write(Red,Green,Blue):
*	This method is provided to set the 
*	Color of the Led
*/
void CoolBoardLed::write(int R, int G, int B)
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoardLed.write()") );
	Serial.println();
	Serial.print( F("R : ") );
	Serial.println(R);
	Serial.print( F("G : ") );
	Serial.println(G);
	Serial.print( F("B : ") );
	Serial.println(B);
	Serial.println();	

#endif

	if(this->ledActive == 1 )
	{
		neoPixelLed->SetPixelColor(0, RgbColor(R, G, B));
		neoPixelLed->Show();
	}

}

/**
*	CoolBoardLed::config():
*	This method is provided to configure
*	the Led Object :	-ledActive=0 : deactivated
*				-ledActive=1 : activated
*	\return true if the configuration done,
*	false otherwise			
*/
bool CoolBoardLed::config()
{

#if DEBUG == 1 
		
	Serial.println( F("Entering CoolBoardLed.config()") );
	Serial.println();

#endif
	
	File coolBoardLedConfig = SPIFFS.open("/coolBoardLedConfig.json", "r");

	if (!coolBoardLedConfig) 
	{
	
	#if DEBUG == 1

		Serial.println( F("failed to read /coolBoardLedConfig.json") );
		Serial.println();

	#endif

		return(false);
	}
	else
	{
		size_t size = coolBoardLedConfig.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		coolBoardLedConfig.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{
		
		#if DEBUG == 1

			Serial.println( F("failed to parse json") );
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
			
			Serial.print(F("jsonBuffer size :"));
			Serial.println(jsonBuffer.size());
			Serial.println();

		#endif
  
			if(json["ledActive"].success() )
			{
				this->ledActive = json["ledActive"]; 
			}
			else
			{
				this->ledActive=this->ledActive;			
			}
			
			json["ledActive"]=this->ledActive;
			coolBoardLedConfig.close();
			
			coolBoardLedConfig = SPIFFS.open("/coolBoardLedConfig.json", "w");
			if(!coolBoardLedConfig)
			{
			
			#if DEBUG == 1 

				Serial.println( F("failed to write to /coolBoardLedConfig.json") );
				Serial.println();

			#endif

				return(false);			
			}

			json.printTo(coolBoardLedConfig);
			coolBoardLedConfig.close();

		#if DEBUG == 1
	
			Serial.println( F("saved Led Config is : ") );
			json.printTo(Serial);
			Serial.println();

		#endif

			return(true); 
		}
	}	

}				

/**
*	CoolBoardLed::printConf():
*	This method is provided to print the
*	Led Object Configuration to the Serial
*	Monitor
*/
void CoolBoardLed::printConf()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolBoardLed.printConf()") );
	Serial.println();

#endif

	Serial.println("Led Configuration");

	Serial.print("ledActive : ");
	Serial.println(ledActive);

	Serial.println();	
}

/**
*	CoolBoardLed::activate():
*	This method is provided to activate the
*	Led Object without the configuration
*	file
*/
void CoolBoardLed::activate()
{

	this->ledActive=1;

}

