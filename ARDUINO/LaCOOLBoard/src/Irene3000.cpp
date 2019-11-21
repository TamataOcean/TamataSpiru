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

#include <Arduino.h>
#include "ArduinoJson.h"
#include <math.h>    
#include "CoolBoardLed.h"	//CoolBoard Led Manager
#include <Irene3000.h>




#define DEBUG 1


/**
*	Irene3000::begin():
*	This method is provided to start the
*	Irene3000 
*/
void Irene3000::startADC()
{
	this->ads.begin();

	delay(2000);
}

/**
*	Irene3000::begin():
*	This method is provided to start the
*	Irene3000 
*/
void Irene3000::begin()
{
	this->ads.begin();

	delay(2000);

#if DEBUG == 1 


	Serial.println( F("Entering Irene3000.begin()") );
	Serial.println();
	
	
	

#endif
	Serial.println(F("Press the button to calibrate the Ph probe "));

	
	delay(2000);

	int bValue=this->readButton();
	if (bValue >= 65000) bValue = 0;
	
	while(bValue<500)
	{
		bValue=this->readButton();
		delay(10);	
	}

	if(  bValue > 20000 )
	{
	

		
		Serial.println(F("calibrating the Ph probe " ) );
		
		Serial.println(F("ph7 calibration for 25 seconds"));
		
		delay(10000);
		
		this->calibratepH7();

		delay(15000);		
	
		this->calibratepH7();

		delay(1000);
		Serial.println(F("ph 7 calibration ok"));
		Serial.println();
	

		Serial.println(F("ph 4 calibration for 25 seconds"));
	
		delay(10000);		

		this->calibratepH4();

		delay(15000);
		
		this->calibratepH4();
	
		delay(1000);		

		Serial.println(F("ph 4 calibration ok"));
		Serial.println();
	


		this->saveParams();
	
	}
}

/**
*	Irene3000:read():
*	This method is provided to read
*	the Irene3000 sensors data
*
*	\return json string of the sensors
*	data
*/
String Irene3000::read()
{

#if DEBUG == 1 
	
	Serial.println( F("Entering Irene3000.read()") );
	Serial.println();

#endif 

	String data;
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	if( !( root.success()) )
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to create json") );
	
	#endif 

		return("");
	}

		
	if(waterTemp.active)
	{
		root["waterTemp"] = this->readTemp();

		if(phProbe.active)
		{
			root["ph"] =this->readPh( root["waterTemp"].as<double>() ) ;
		}

	}

	if(adc2.active)
	{
		root[adc2.type] =this->readADSChannel2(adc2.gain);
	}
	
	root.printTo(data);
	
#if DEBUG == 1 

	Serial.println( F("Irene data : ") );
	Serial.println(data);
	Serial.println();

	Serial.print(F("jsonBuffer size: "));
	Serial.println(jsonBuffer.size());
	Serial.println();


#endif
	
	return(data);
	
	

}

/**
*	Irene3000::config():
*	This method is provided to configure the
*	Irene3000 shield through a configuration file
*
*	\return true if successful,false otherwise
*/
bool Irene3000::config()
{

#if DEBUG == 1 

	Serial.println( F("Entering Irene3000.config()") );
	Serial.println();

#endif

	File irene3000Config = SPIFFS.open("/irene3000Config.json", "r");

	if (!irene3000Config) 
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to read /irene3000Config.json") );
		Serial.println();
	
	#endif

		return(false);
	}
	else
	{
		size_t size = irene3000Config.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);
	        uint16_t tempGain;
		irene3000Config.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{
		
		#if DEBUG == 1 

			Serial.println( F("failed to parse json ") );
			Serial.println();
		
		#endif
			
			return(false);
		} 
		else
		{
		
		#if DEBUG == 1 
 	
			Serial.println( F("read configuration file ") );
			json.printTo(Serial);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();

		

		#endif 			
			if(json["waterTemp"]["active"].success() )
			{			
				this->waterTemp.active = json["waterTemp"]["active"]; 
			}
			else
			{
				this->waterTemp.active=this->waterTemp.active;
			}
			json["waterTemp"]["active"]=this->waterTemp.active;

			
			if(json["phProbe"]["active"].success())
			{
				this->phProbe.active=json["phProbe"]["active"];
			}
			else
			{
				this->phProbe.active=this->phProbe.active;
			}
			json["phProbe"]["active"]=this->phProbe.active;

			
			if(json["adc2"]["active"].success() )
			{
				this->adc2.active=json["adc2"]["active"];
			}
			else
			{
				this->adc2.active=this->adc2.active;
			}
			json["adc2"]["active"]=this->adc2.active;

			
			if(json["adc2"]["gain"].success() )
			{			
				tempGain=json["adc2"]["gain"];
				this->adc2.gain=this->gainConvert(tempGain);
			}
			else
			{
				this->adc2.gain=this->adc2.gain;
			}
			json["adc2"]["gain"]=this->adc2.gain;

			
			if(json["adc2"]["type"].success() )
			{
				this->adc2.type=json["adc2"]["type"].as<String>(); 
			}
			else
			{
				this->adc2.type=this->adc2.type;
			}
			json["adc2"]["type"]=this->adc2.type;


			if(json["pH7Cal"].success())
			{
				this->params.pH7Cal=json["pH7Cal"];			
			}
			else
			{
				this->params.pH7Cal=this->params.pH7Cal;
			}
			json["pH7Cal"]=this->params.pH7Cal;

			
			if(json["pH4Cal"].success())
			{
				this->params.pH4Cal=json["pH4Cal"];			
			}
			else
			{
				this->params.pH4Cal=this->params.pH4Cal;
			}
			json["pH4Cal"]=this->params.pH4Cal;


			if(json["pHStep"].success())
			{
				this->params.pHStep=json["pHStep"];			
			}
			else
			{
				this->params.pHStep=this->params.pHStep;
			}
			json["pHStep"]=this->params.pHStep;


			irene3000Config.close();
			irene3000Config = SPIFFS.open("/irene3000Config.json", "w");

			if(!irene3000Config)
			{
			
			#if DEBUG == 1

				Serial.println( F("failed to write to /irene3000Config.json") );
				Serial.println();
			
			#endif 

				return(false);
			}

			json.printTo(irene3000Config);
			irene3000Config.close();
			
		#if DEBUG == 1 

			Serial.println( F("saved configuration file :")  );
			json.printTo(Serial);
			Serial.println();
		
		#endif

			return(true); 
		}
	}	

}

/**
*	Irene3000::printConf():
*	This method is provided to print the configuration
*	to the Serial Monitor
*/
void Irene3000::printConf()
{

#if DEBUG == 1 

	Serial.println( F("Entering Irene3000.printConf()") );
	Serial.println();

#endif 

	Serial.println("Irene Configuration ");

	Serial.print("waterTemp.active : ");
	Serial.println(waterTemp.active);

	Serial.print("waterTemp.gain : ");
	Serial.println(waterTemp.gain,HEX);	

	Serial.print("phProbe.active : ");
	Serial.println(phProbe.active);

	Serial.print("phProbe.gain : ");
	Serial.println(phProbe.gain,HEX);
	
	Serial.print("adc2.active : ");
	Serial.println(adc2.active);

	Serial.print("adc2.gain : ");
	Serial.println(adc2.gain,HEX);

	Serial.print("adc2.type : ");
	Serial.println(adc2.type);

	Serial.println();
}

/**
*	Irene3000::readButton():
*	This method is provided to read the
*	Irene3000 button
*
*	\return the button value
*/
int Irene3000::readButton()
{

#if DEBUG == 1 

	Serial.println( F("Entering Irene3000.readButton()" ) );
	Serial.println();

#endif 

	this->setGain(GAIN_TWOTHIRDS);
	int result =this->ads.readADC_SingleEnded(button);
#if DEBUG == 1
	
	Serial.println( F("button value : ") );
	Serial.println( result );

#endif 

	return( result );
	
}

/**
*	Irene3000::setGain(gain):
*	This method is provided to set the
*	ADS chip gain
*/
void Irene3000::setGain(adsGain_t gain)
{

#if DEBUG == 1	

	Serial.println( F("Entering Irene3000.setGain()") );
	Serial.println();

#endif

	this->ads.setGain(gain);
}

/**
*	Irene3000::readADSChannel2(gain):
*	This method is provided to read from
*	the ADS channel 2 .
*	ADS Channel 2 is free and the user can connect
*	another analog sensor to it.
*
*	\return the ADS Channel 2 value
*/
int Irene3000::readADSChannel2(adsGain_t gain)
{	

#if DEBUG == 1 
	
	Serial.println( F("Entering Irene3000.readADSChannel2()") );
	Serial.println();

#endif

	this->setGain(gain);
	int result = this->ads.readADC_SingleEnded(freeAdc);
#if DEBUG == 1 
	
	Serial.println( F("adc2 value : ") );
	Serial.println( );
	Serial.println();

#endif

	return( result ) ;
}

/**
*	Irene3000::readPh(double t):
*	This method is provided to read the PH probe
*	note that for the best results, PH must be 
*	correlated to Temperature.
*
*	\return the PH probe value
*/
float Irene3000::readPh(double t)
{

#if DEBUG == 1 

	Serial.println( F("Entering Irene3000.readPh()") );
	Serial.println();

#endif 

	this->setGain(GAIN_FOUR);
	int adcR=ads.readADC_SingleEnded(ph);
	double Voltage =  REFERENCE_VOLTAGE_GAIN_4 * ( adcR ) / ADC_MAXIMUM_VALUE;

	float miliVolts = Voltage * 1000;
	float temporary = ((((vRef * (float)params.pH7Cal) / 32767) * 1000) - miliVolts) / opampGain;
	
	float phT=7 - (temporary / params.pHStep);

	float ph25= ( phT / ( 1 + 0.009*( t - 25 ) ) );

#if DEBUG == 1 

	Serial.println( F("ph is : ") );
	Serial.println( phT ) ;
	
	Serial.println(F("corrected ph to 25°C is : "));
	Serial.println(ph25);

#endif 
	if(isnan(ph25) )
	{
		return(-42);	
	}

	return(ph25);

}

/**
*	Irene3000::readTemp(gain):
*	This method is provided to read
*	the Temeperature probe
*
*	\return the Temperature probe value
*/
double Irene3000::readTemp()
{

#if DEBUG == 1 

	Serial.println( F("Entering Irene3000.readTemp()") );
	Serial.println();

#endif

	const double A = 3.9083E-3;
	const double B = -5.775E-7;
	double T;

	this->setGain(GAIN_EIGHT);

	double adc0 = ads.readADC_SingleEnded(temp);


	double R = ( ( adc0 * V_GAIN_8 ) / 0.095 ) / 1000 ;

	T = 0.0 - A;
	T += sqrt((A * A) - 4.0 * B * (1.0 - R));
	T /= (2.0 * B);

	if (T > 0 && T < 200) 
	{

	#if DEBUG == 1 

		Serial.print( F(" temperature : ") );
		Serial.println(T);
		Serial.println();
	
	#endif 
		if(isnan(T))
		{
			return(-300);			
		}

		return T;
	}
	else 
	{
		T = 0.0 - A;
		T -= sqrt((A * A) - 4.0 * B * (1.0 - R));
		T /= (2.0 * B);
	
	#if DEBUG == 1 
	
		Serial.println( F("temperature : ") );
		Serial.println(T);
		Serial.println();
	
	#endif
		if(isnan(T))
		{
			return(-400);			
		}

		return T;
	}

}


/**
*	Irene3000::calibratepH7():
*	This method is provided to calibrate the
*	PH probe to 7
*/
void Irene3000::calibratepH7()
{

#if DEBUG == 1 

	Serial.println( F("Entering Irene3000.calibratepH7() ") );
	Serial.println();
	
	

#endif 
	delay(1000);

  	this->setGain(GAIN_FOUR);
        
	this->params.pH7Cal = ads.readADC_SingleEnded(ph);
 
	this->calcpHSlope();


}

/**
*	Irene3000::calibratepH4():
*	This method is provided to calibrate the
*	PH probe to 4
*/
void Irene3000::calibratepH4()
{

#if DEBUG == 1 

	Serial.println( F("Entering Irene3000.calibraph4()") );
	Serial.println();

#endif 
	delay(1000);

	this->setGain(GAIN_FOUR);

	this->params.pH4Cal =  ads.readADC_SingleEnded(ph);

	this->calcpHSlope();



}

/**
*	Irene3000::calcpHSlop():
*	This method is provided to calculate
*	th PH slope
*/
void Irene3000::calcpHSlope ()
{

#if DEBUG == 1 

	Serial.println( F("Entering Irene3000.calcpHSlope()") );
	Serial.println();

#endif 

	params.pHStep = ((((vRef * (float)(params.pH7Cal - params.pH4Cal)) / 32767) * 1000) / opampGain) / 3;

 
}

/**
*	Irene3000::resetParams():
*	This method is provided to reset
*	the PH configuration, 
*	assuming Ideal configuration
*/
void Irene3000::resetParams(void)
{

#if DEBUG == 1 

	Serial.println( F("Entering Irene3000.resetParams()") );
	Serial.println();

#endif 

	//Restore to default set of parameters!
	params.WriteCheck = Write_Check;
	params.pH7Cal = 16384; //assume ideal probe and amp conditions 1/2 of 4096
	params.pH4Cal = 8192; //using ideal probe slope we end up this many 12bit units away on the 4 scale
	params.pHStep = 59.16;//ideal probe slope


}

/**
*	Irene3000::gainConvert( gain : { 0.67 ,1,2,4,8,16 } )
*	This method is provided to convert the gain to
*	Internal Constants
*
*	\return internal representation of the ADS gain
*/
adsGain_t Irene3000::gainConvert(uint16_t tempGain)
{

#if DEBUG == 1 

	Serial.println( F("Entering Irene3000.gainConvert()") );
	Serial.println();

#endif 
	
	switch(tempGain)
	{
		case(1): return (GAIN_ONE);
		case(2) : return(GAIN_TWO);
		case(4): return(GAIN_FOUR) ;   
		case(8):return(GAIN_EIGHT)  ;  
		case(16):return(GAIN_SIXTEEN);
		default: return(GAIN_TWOTHIRDS); 	
	}

	return(GAIN_ONE);

}


bool Irene3000::saveParams()
{

#if DEBUG == 1
		
	Serial.println(F("Entering Irene3000.saveParams()"));
	Serial.println();
	
	Serial.println(F("saving new params"));
#endif
	
	//open file
	File irene3000Config = SPIFFS.open("/irene3000Config.json", "r");

	if (!irene3000Config) 
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to read /irene3000Config.json") );
		Serial.println();
	
	#endif

		return(false);
	}
	else
	{
		//read file into json
		size_t size = irene3000Config.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);
	        irene3000Config.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{
		
		#if DEBUG == 1 

			Serial.println( F("failed to parse json ") );
			Serial.println();
		
		#endif
			
			return(false);
		} 
		else
		{
		
		#if DEBUG == 1 
 	
			Serial.println( F("read configuration file ") );
			json.printTo(Serial);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();

		

		#endif 			
	
			//add params to json
			json["pH7Cal"]=this->params.pH7Cal;
			json["pH4Cal"]=this->params.pH4Cal;
			json["pHStep"]=this->params.pHStep;			
			
			irene3000Config.close();
			
			
			//save json to file
			irene3000Config = SPIFFS.open("/irene3000Config.json", "w");

			if(!irene3000Config)
			{
			
			#if DEBUG == 1

				Serial.println( F("failed to write to /irene3000Config.json") );
				Serial.println();
			
			#endif 

				return(false);
			}

			json.printTo(irene3000Config);
			irene3000Config.close();
			
		#if DEBUG == 1 

			Serial.println( F("saved configuration file :")  );
			json.printTo(Serial);
			Serial.println();
		
		#endif

			return(true); 
		}
	}	

}

