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

#include"Arduino.h"  

#include"OneWire.h"

#include"ExternalSensors.h"

#include "ArduinoJson.h"

#include"ExternalSensor.h"

#define DEBUG 0

OneWire oneWire(0);


/**
*	ExternalSensors::begin():
*	This method is provided to initialise
*	the external sensors. 
*/
void ExternalSensors::begin() 
{

#if DEBUG == 1

	Serial.println( F("Enter ExternalSensors.begin()") );
	Serial.println();

#endif 

	for(int i=0;i< this->sensorsNumber ; i++)
	{
		if( (sensors[i].reference) == "NDIR_I2C" )
		{	
			std::unique_ptr< ExternalSensor<NDIR_I2C> > sensorCO2(new ExternalSensor<NDIR_I2C>( this->sensors[i].address) );


			sensors[i].exSensor= sensorCO2.release();
			sensors[i].exSensor->begin();
			sensors[i].exSensor->read();

		}
		else if( (sensors[i].reference) == "DallasTemperature")
		{

			std::unique_ptr< ExternalSensor<DallasTemperature> > dallasTemp(new ExternalSensor<DallasTemperature> (&oneWire));
			 
			sensors[i].exSensor=dallasTemp.release();
			sensors[i].exSensor->begin();
			sensors[i].exSensor->read();
		}
		else if( (sensors[i].reference) == "Adafruit_TCS34725")
		{
			int16_t r, g, b, c, colorTemp, lux;

			std::unique_ptr< ExternalSensor<Adafruit_TCS34725> > rgbSensor(new ExternalSensor<Adafruit_TCS34725> ());
			 
			sensors[i].exSensor=rgbSensor.release();
			sensors[i].exSensor->begin();
			sensors[i].exSensor->read(&r,&g,&b,&c,&colorTemp,&lux);
		}

		else if( (sensors[i].reference) == "Adafruit_CCS811")
		{
			int16_t C02, VOT;
			float Temp;

			std::unique_ptr< ExternalSensor<Adafruit_CCS811> > aqSensor(new ExternalSensor<Adafruit_CCS811> (this->sensors[i].address));
			sensors[i].exSensor=aqSensor.release();
			sensors[i].exSensor->begin();
			sensors[i].exSensor->read(&C02, &VOT, &Temp);
		}

		else if( (sensors[i].reference) == "Adafruit_ADS1015")
		{
			int16_t channel0, channel1, channel2, channel3, diff01, diff23;
			int16_t gain0, gain1, gain2, gain3;

			std::unique_ptr< ExternalSensor<Adafruit_ADS1015> > analogI2C(new ExternalSensor<Adafruit_ADS1015> (this->sensors[i].address));
			 
			sensors[i].exSensor=analogI2C.release();
			sensors[i].exSensor->begin();
			sensors[i].exSensor->read(&channel0, &gain0, &channel1, &gain1, &channel2, &gain2, &channel3, &gain3);

		}

		else if( (sensors[i].reference) == "Adafruit_ADS1115")
		{
			//Serial.print("ADS1115 !!!  address: ");
			//Serial.println(this->sensors[i].address);
			int16_t channel0, channel1, channel2, channel3, diff01, diff23;
			int16_t gain0, gain1, gain2, gain3;

			std::unique_ptr< ExternalSensor<Adafruit_ADS1115> > analogI2C(new ExternalSensor<Adafruit_ADS1115> (this->sensors[i].address));
			 
			sensors[i].exSensor=analogI2C.release();
			//sensors[i].exSensor->begin();
			sensors[i].exSensor->read(&channel0, &gain0, &channel1, &gain1, &channel2, &gain2, &channel3, &gain3);
		}

		else if( (sensors[i].reference) == "CoolGauge")
		{
			uint32_t A, B, C;

			std::unique_ptr< ExternalSensor<Gauges> > gauge(new ExternalSensor<Gauges> ());
			 
			sensors[i].exSensor=gauge.release();
			//sensors[i].exSensor->begin();
			sensors[i].exSensor->read(&A, &B, &C);
		}
		
	}
}

/**
*	ExternalSensors::read():	
*	This method is provided to
*	read the data from the external sensors
*
*	\return json string that contains the
*	sensors data
*/
String ExternalSensors::read()
{

#if DEBUG == 1

	Serial.println( F("Entering ExternalSensors.read()") );
	Serial.println();

#endif 

	String data;
	DynamicJsonBuffer  jsonBuffer ;
	JsonObject& root = jsonBuffer.createObject();

	if(!root.success() )
	{
 
	#if DEBUG == 1

		Serial.println( F("failed to create json ") );
	
	#endif 

		return("00");
	}
	else
	{
		if(sensorsNumber>0)
		{
			for(int i=0;i<sensorsNumber;i++)
			{
				if(sensors[i].exSensor != NULL )
				{
					if(sensors[i].reference=="Adafruit_TCS34725")
					{
						int16_t r, g, b, c, colorTemp, lux;

			  			sensors[i].exSensor->read(&r,&g,&b,&c,&colorTemp,&lux);

			  			/* ***************************************************************************************
			  			/* ISSUE - Mqtt disconnect when CoolBoard Start sending config to Mqtt.. Cause unknown...*
			  			/* *************************************************************************************** 
			  			//Calculate to export RGB on 256bit ( interpretable for @453423 color)
						if (sensors[i].kind0 == "HEX256") {
						        #if DEBUG == 1
						        Serial.println("------- HEX256 Config asked");
						        Serial.println("---------------------------");
						        #endif
						        // set to false if using a common cathode LED
						        bool commonAnode=true;
						        byte gammatable[256];
						        for (int i=0; i<256; i++) {
						        float x = i;
						            x /= 255;
						            x = pow(x, 2.5);
						            x *= 255;
						            if (commonAnode) {
						              gammatable[i] = 255 - x;
						            } else {
						              gammatable[i] = x;      
						            }
						    	}
						    	uint32_t sum = c;
								  float _r, _g, _b;
								_r = r; _r /= sum;
								_g = g; _g /= sum;
								_b = b; _b /= sum;
								_r *= 256; _g *= 256; _b *= 256;
								r = _r; g= _g; b= _b;
								Serial.print("\t");
								Serial.println("RGB Hex value :");
								Serial.println((int)_r, HEX); Serial.println((int)_g, HEX); Serial.println((int)_b, HEX);
								Serial.println("RGB Dec value :");
								Serial.println((int)r); Serial.println((int)g); Serial.println((int)b);
						}*/
						
			  			root[sensors[i].kind0] = r;
						root[sensors[i].kind1] = g;
						root[sensors[i].kind2] = b;
						root[sensors[i].kind3] = colorTemp;
						//root[sensors[i].kind4] = lux;
						
					}
					else if(sensors[i].reference=="Adafruit_CCS811")
					{
						int16_t C, V ;
						float T;

			  			sensors[i].exSensor->read(&C,&V,&T);

			  			root[sensors[i].kind0] = C;
						root[sensors[i].kind1] = V;
						root[sensors[i].kind2] = T;
					}
					else if((sensors[i].reference=="Adafruit_ADS1015" ) || (sensors[i].reference=="Adafruit_ADS1115" ) )
					{
						int16_t channel0, channel1, channel2, channel3, diff01, diff23;
						int16_t gain0, gain1, gain2, gain3;

						sensors[i].exSensor->read(&channel0, &gain0, &channel1, &gain1, &channel2, &gain2, &channel3, &gain3);
						gain0 = gain0/512;
						gain1 = gain1/512;
						gain2 = gain2/512;
						gain3 = gain3/512;
						root["0_" + sensors[i].kind0] = channel0;
						root["G0_" + sensors[i].kind0] = gain0;
						root["1_" + sensors[i].kind1] = channel1;
						root["G1_" + sensors[i].kind1] = gain1;
						root["2_" + sensors[i].kind2] = channel2;
						root["G2_" + sensors[i].kind2] = gain2;
						root["3_" + sensors[i].kind3] = channel3;
						root["G3_" + sensors[i].kind3] = gain3;
					}
					else if(sensors[i].reference=="CoolGauge")
					{
						uint32_t A, B, C;

			  			sensors[i].exSensor->read(&A, &B, &C);

			  			root[sensors[i].kind0] = A;
						root[sensors[i].kind1] = B;
						root[sensors[i].kind2] = C;
					}
					else
					{
						root[sensors[i].type]=sensors[i].exSensor->read();
					}	 	
				}
			
			#if DEBUG == 1
				else
				{
					Serial.println(F("null pointer "));
				}
			#endif	
			}
		}	
		
		root.printTo(data);
	
	#if DEBUG == 1

		Serial.println( F("sensors data :") );
		Serial.println(data);
		Serial.println();

		Serial.print(F("jsonBuffer size: "));
		Serial.println(jsonBuffer.size());
		Serial.println();

	
	#endif
		return(data);
	}

}

/**
*	ExternalSensors::config():
*	This method is provided to configure
*	the externalSensors through a configuration
*	file
*
*	\return true if successful,false otherwise
*/
bool ExternalSensors::config()
{
	//read config file
	//update data
	File externalSensorsConfig = SPIFFS.open("/externalSensorsConfig.json", "r");

	if (!externalSensorsConfig) 
	{
	
	#if DEBUG == 1
		
		Serial.println( F("failed to read /externalSensorsConfig.json") );
		Serial.println();
	
	#endif
		
		return(false);
	}
	else
	{
		size_t size = externalSensorsConfig.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		externalSensorsConfig.readBytes(buf.get(), size);
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
	
			Serial.println( F("configuration json is : ") );
			json.printTo(Serial);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();

		
		#endif			
			if(json["sensorsNumber"]!=NULL)
			{
				this->sensorsNumber = json["sensorsNumber"];
				
				

				for(int i=0;i<sensorsNumber;i++)
				{	String name="sensor"+String(i);
					
					if(json[name].success())
					{  
						JsonObject& sensorJson=json[name];
						
						if(sensorJson["reference"].success() )
						{  
							this->sensors[i].reference =sensorJson["reference"].as<String>();
						}
						else
						{
							this->sensors[i].reference=this->sensors[i].reference;							
									
						}
						sensorJson["reference"]=this->sensors[i].reference;

					
						if(sensorJson["type"].success() )
						{					
							this->sensors[i].type=sensorJson["type"].as<String>();
						}
						else
						{
							this->sensors[i].type=this->sensors[i].type;

						}
						sensorJson["type"]=this->sensors[i].type;
					
						if(sensorJson["address"].success() )
						{					
							this->sensors[i].address=sensorJson["address"];
						}
						else
						{	
							this->sensors[i].address=this->sensors[i].address;

						}
						sensorJson["address"]=this->sensors[i].address;
						


						if(sensorJson["kind0"].success() )
						{					
							this->sensors[i].kind0=sensorJson["kind0"].as<String>();
						}
						else
						{	
							this->sensors[i].kind0=this->sensors[i].kind0;

						}
						sensorJson["kind0"]=this->sensors[i].kind0;	


						if(sensorJson["kind1"].success() )
						{					
							this->sensors[i].kind1=sensorJson["kind1"].as<String>();
						}
						else
						{	
							this->sensors[i].kind1=this->sensors[i].kind1;

						}
						sensorJson["kind1"]=this->sensors[i].kind1;	

						if(sensorJson["kind2"].success() )
						{					
							this->sensors[i].kind2=sensorJson["kind2"].as<String>();
						}
						else
						{	
							this->sensors[i].kind2=this->sensors[i].kind2;

						}
						sensorJson["kind2"]=this->sensors[i].kind2;	

						if(sensorJson["kind3"].success() )
						{					
							this->sensors[i].kind3=sensorJson["kind3"].as<String>();
						}
						else
						{	
							this->sensors[i].kind3=this->sensors[i].kind3;

						}
						sensorJson["kind3"]=this->sensors[i].kind3;	
/*	ISSUE --------------
						if(sensorJson["kind4"].success() )
						{					
							this->sensors[i].kind4=sensorJson["kind4"].as<String>();
						}
						else
						{	
							this->sensors[i].kind4=this->sensors[i].kind4;

						}
						sensorJson["kind4"]=this->sensors[i].kind4;					
*/	
					}
					else
					{
						this->sensors[i]=this->sensors[i];					
					}
								        	
					json[name]["reference"]=this->sensors[i].reference;
					json[name]["type"]=this->sensors[i].type;
					json[name]["address"]=this->sensors[i].address;
					json[name]["kind0"]=this->sensors[i].kind0;
					json[name]["kind1"]=this->sensors[i].kind1;
					json[name]["kind2"]=this->sensors[i].kind2;
					json[name]["kind3"]=this->sensors[i].kind3;
//					json[name]["kind4"]=this->sensors[i].kind4;
				}
 
			}
			else
			{
				this->sensorsNumber=this->sensorsNumber;
			}
			json["sensorsNumber"]=this->sensorsNumber;

			externalSensorsConfig.close();
			externalSensorsConfig = SPIFFS.open("/externalSensorsConfig.json", "w");

			if(!externalSensorsConfig)
			{
			
			#if DEBUG == 1 

				Serial.println( F("failed to write to /externalSensorsConfig.json") );
				Serial.println();
			
			#endif

				return(false);
			}
			
			json.printTo(externalSensorsConfig);
			externalSensorsConfig.close();
			
		#if DEBUG == 1 

			Serial.println( F("saved configuration is : ") );
			json.printTo(Serial);
			Serial.println();
		
		#endif

			return(true); 
		}
	}	
	



}

/**
*	ExternalSensors::config(String reference[],String type[],uint8_t address[],int sensorsNumber):
*	This method is provided to configure
*	the externalSensors without a configuration
*	file
*
*	\return true if successful,false otherwise
*/
bool ExternalSensors::config(String reference[],String type[],uint8_t address[],int sensorsNumber)
{

#if DEBUG == 1

	Serial.println( F("Entering ExternalSensors.conf(reference[], type[], address[], sensorsNumber)") );
	Serial.println();

#endif 	
	if(sensorsNumber>50)
	{
	
	#if DEBUG == 1
	
		Serial.println(F("you can't add more than 50 sensors"));	
	
	#endif	
	
		return(false);
	}

	this->sensorsNumber=sensorsNumber;
	
	for(int i=0;i<sensorsNumber;i++)
	{
	
		this->sensors[i].reference=reference[i];
		
		this->sensors[i].type=type[i];

		this->sensors[i].address=address[i];
	
	}
	
	return(true);

}

/**
*	ExternalSensors::printConf():
*	This method is provided to print the
*	configuration to the Serial Monitor
*/
void ExternalSensors::printConf()
{

#if DEBUG == 1

	Serial.println( F("Entering ExternalSensors.printConf()") );
	Serial.println();

#endif 

	Serial.println("External Sensors configuration ");

	Serial.print("sensorsNumber : ");
	Serial.println(sensorsNumber);

	for(int i=0;i<sensorsNumber;i++)
	{
		Serial.print("sensor ");
		Serial.print(i);
		Serial.print(" reference : ");
		Serial.println(this->sensors[i].reference);

		Serial.print("sensor ");
		Serial.print(i);
		Serial.print(" type : ");
		Serial.println(this->sensors[i].type);
				
		Serial.print("sensor ");
		Serial.print(i);
		Serial.print(" address : ");
		Serial.println(this->sensors[i].address);
	
	}
}

