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
#include <stdint.h>        
#include "ArduinoJson.h"
#include "CoolBoardSensors.h"


#define DEBUG 0


/**
*	CoolBoardSensors::CoolBoardSensors():
*	This Constructor is provided to 
*	init the different used pins
*/
CoolBoardSensors::CoolBoardSensors()
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoardSensors Constructor") );
	Serial.println();

#endif
	
	pinMode(AnMplex, OUTPUT);                //Declare Analog Multiplexer OUTPUT
	pinMode(EnMoisture, OUTPUT);             //Declare Moisture enable Pin
	digitalWrite(EnMoisture, HIGH);			 //Prevent Wearing on the soil moisture fork


}


/**
*	CoolBoardSensors::allActive():
*	This method is provided to allow
*	activation of all the sensor board sensors
*	without passing by the configuration file/method
*/
void CoolBoardSensors::allActive()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolBoardSensors.allActive()") );
	Serial.println();

#endif
	
	this->lightDataActive.visible=1;
	this->lightDataActive.ir=1;
	this->lightDataActive.uv=1;	

	this->airDataActive.temperature=1;
	this->airDataActive.humidity=1;
	this->airDataActive.pressure=1;


	this->vbatActive=1;

	this->soilMoistureActive=1;
	


}


/**
*	CoolBoardSensors::begin():
*	This method is provided to start the
*	sensors that are on the sensor board
*/
void CoolBoardSensors::begin()
{  

#if DEBUG == 1 
     
	Serial.println( F("Entering CoolBoardSensors.begin()") );
	Serial.println();

#endif

	while (!lightSensor.Begin()) 
	{
	
	#if DEBUG == 1

		Serial.println( F("Si1145 is not ready!  1 second") );

	#endif

		delay(1000);
  	}
	 
	this->setEnvSensorSettings();

	delay(100);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.

	this->envSensor.begin();

	delay(1000);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.

#if DEBUG == 1 
	
	Serial.print( F("BME280 begin answer is :") );
	Serial.println(envSensor.begin(), HEX);
	Serial.println();

#endif

#if DEBUG == 0

	Serial.println( F("Onboard Sensors : OK"));
	Serial.println();

#endif

}

/**
*	CoolBoardSensors::end():
*	This method is provided to end
*	the sensors on the sensor board
*/
void CoolBoardSensors::end()
{

#if DEBUG == 1 	
	Serial.println( F("Entering CoolBoardSensors.end()") );
	Serial.println();

#endif

	lightSensor.DeInit();

}

/**
*	CoolBoardSensors::read():
*	This method is provided to return the
*	data read by the sensor board
*
*	\return a json string containing the 
*	sensors data
**/
String CoolBoardSensors::read()
{

#if DEBUG == 1 
	
	Serial.println( F("Entering CoolBoardSensors.read()") );
	Serial.println();

#endif

#if DEBUG == 0

	Serial.println( F("Reading Sensors..."));

#endif

	String data;
	DynamicJsonBuffer  jsonBuffer ;
	JsonObject& root = jsonBuffer.createObject();

	delay(100);
	//light data
	if(lightDataActive.visible)
	{	
		//SI1145 Response Reg Value when VIS Overflow
		if(lightSensor.ReadResponseReg()== CoolSI114X_VIS_OVERFLOW )
		{
			root["visibleLight"] ="overflow";

			//send NoP Command to SI1145 to clear overflow value
			lightSensor.WriteParamData(CoolSI114X_COMMAND,CoolSI114X_NOP);
		}
		else
		{

			root["visibleLight"] =lightSensor.ReadVisible() ;

		}
	}
	
	if(lightDataActive.ir)
	{
		//SI1145 Response Reg Value when IR Overflow
		if(lightSensor.ReadResponseReg()==CoolSI114X_IR_OVERFLOW )
		{
			root["infraRed"] ="overflow";

			//send NoP Command to SI1145 to clear overflow value
			lightSensor.WriteParamData(CoolSI114X_COMMAND,CoolSI114X_NOP);
		}
		else
		{
			root["infraRed"] = lightSensor.ReadIR();
		}
	}

	if(lightDataActive.uv)
	{
		//SI1145 Response Reg Value when UV Overflow
		if(lightSensor.ReadResponseReg()==CoolSI114X_UV_OVERFLOW)
		{
			root["ultraViolet"] ="overflow";

			//send NoP Command to SI1145 to clear overflow value
			lightSensor.WriteParamData(CoolSI114X_COMMAND,CoolSI114X_NOP);
		}
		else
		{
			float tempUV = (float)lightSensor.ReadUV()/100 ;
			root["ultraViolet"] = tempUV;
		}
	}
	
	//BME280 data
	if(airDataActive.temperature)
	{

		//wait for BME280 to finish data conversion( status reg bit3 ==0)
		while((envSensor.readRegister(BME280_STAT_REG) & 0x10 ) != 0 )
		{
			yield();
		}
		root["Temperature"]=envSensor.readTempC();
	}
	if(airDataActive.pressure)	
	{

		//wait for BME280 to finish data conversion( status reg bit3 ==0)
		while((envSensor.readRegister(BME280_STAT_REG) & 0x10 ) != 0 )
		{
			yield();
		}
		root["Pressure"] =envSensor.readFloatPressure();
	}
	
		
	if(airDataActive.humidity)	
	{
		
		//wait for BME280 to finish data conversion( status reg bit3 ==0)
		while((envSensor.readRegister(BME280_STAT_REG) & 0x10 ) != 0 )
		{
			yield();
		}
		root["Humidity"] =envSensor.readFloatHumidity() ;
	}	
	

	
	//Vbat
	if(vbatActive)	
	{	
		root["Vbat"]=this->readVBat();
	}
	
	//earth Moisture
	if(soilMoistureActive)
	{	
		root["soilMoisture"]=this->readMoisture();
	}
	
	
	root.printTo(data);
#if DEBUG == 1
	Serial.println( F("CoolBoardSensors data is :") );
	root.printTo(Serial);
	Serial.println();
	Serial.print(F("jsonBuffer size: "));
	Serial.println(jsonBuffer.size());
	Serial.println();

#endif

	return(data);	
	

}

/**
*	CoolBoardSensors::config():
*	This method is provided to configure the
*	sensor board :	-activate   1
*			-deactivate 0
*
*	\return true if configuration is successful,
*	false otherwise
*/
bool CoolBoardSensors::config()
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoardSensors.config()") );
	Serial.println();

#endif

#if DEBUG == 0

	Serial.println( F("Reading Sensor Configuration..."));

#endif
	//read config file
	//update data
	File coolBoardSensorsConfig = SPIFFS.open("/coolBoardSensorsConfig.json", "r");

	if (!coolBoardSensorsConfig) 
	{
	
		Serial.println( F("failed to read /coolBoardSensorsConfig.json") );
		Serial.println();

		return(false);
	}
	else
	{
		size_t size = coolBoardSensorsConfig.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		coolBoardSensorsConfig.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{

			Serial.println( F("failed to parse coolBoardSensorsConfig json") );
			Serial.println();
	
			return(false);
		} 
		else
		{

		#if DEBUG == 1

			Serial.println( F("Configuration Json is :") );
			json.printTo(Serial);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();
		
		#endif
			
			if(json["BME280"]["temperature"].success() )
			{			
				this->airDataActive.temperature=json["BME280"]["temperature"];
			}
			else
			{
				this->airDataActive.temperature=this->airDataActive.temperature;			
			}
			json["BME280"]["temperature"]=this->airDataActive.temperature;
			
			
			if(json["BME280"]["humidity"].success() )
			{			
			
				this->airDataActive.humidity=json["BME280"]["humidity"];
			}
			else
			{
				this->airDataActive.humidity=this->airDataActive.humidity;
			}
			json["BME280"]["humidity"]=this->airDataActive.humidity;
			
			
			if(json["BME280"]["pressure"].success() )
			{
				this->airDataActive.pressure=json["BME280"]["pressure"];
			}
			else
			{
				this->airDataActive.pressure=this->airDataActive.pressure;
			}
			json["BME280"]["pressure"]=this->airDataActive.pressure;

			
			if(json["SI114X"]["visible"].success() )
			{
				this->lightDataActive.visible=json["SI114X"]["visible"];
			}
			else
			{
				this->lightDataActive.visible=this->lightDataActive.visible;
			}
			json["SI114X"]["visible"]=this->lightDataActive.visible;
			
			
			if(json["SI114X"]["ir"].success() )
			{			
				this->lightDataActive.ir=json["SI114X"]["ir"];
			}
			else
			{
				this->lightDataActive.ir=this->lightDataActive.ir;
			}
			json["SI114X"]["ir"]=this->lightDataActive.ir;

			
			if(json["SI114X"]["uv"].success() )			
			{			
				this->lightDataActive.uv=json["SI114X"]["uv"];
			}
			else
			{
				this->lightDataActive.uv=this->lightDataActive.uv;
			}
			json["SI114X"]["uv"]=this->lightDataActive.uv;


			if(json["vbat"].success() )
			{
				this->vbatActive=json["vbat"];
			}
			else
			{
				this->vbatActive=this->vbatActive;
			}
			json["vbat"]=this->vbatActive;

			
			if(json["soilMoisture"].success() )
			{			
				this->soilMoistureActive= json["soilMoisture"];
			}
			else
			{
				this->soilMoistureActive=this->soilMoistureActive;
			}
			json["soilMoisture"]=this->soilMoistureActive;

			coolBoardSensorsConfig.close();			
			coolBoardSensorsConfig = SPIFFS.open("/coolBoardSensorsConfig.json", "w");			
			if(!coolBoardSensorsConfig)
			{

				Serial.println( F("failed to write to /coolBoardSensorsConfig.json") );
				Serial.println();

				return(false);			
			}  

			json.printTo(coolBoardSensorsConfig);
			coolBoardSensorsConfig.close();			
			
		#if DEBUG == 1

			Serial.println( F("Saved Configuration Json is : ") );
			json.printTo(Serial);
			Serial.println();
		
		#endif

		#if DEBUG == 0
			Serial.println( F("Configuration loaded : OK"));
		#endif

			return(true); 
		}
	}	

}


/**
*	CoolBoardSensors::printConf():
*	This method is provided to print the 
*	configuration to the Serial Monitor
*/
void CoolBoardSensors::printConf()
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoardSensors.printConf()") );
	Serial.println();

#endif

	Serial.println( F("Sensors Configuration : "));
	
	Serial.print( F("airDataActive.temperature : "));
	Serial.println(this->airDataActive.temperature);

	Serial.print( F("airDataActive.humidity : "));
	Serial.println(airDataActive.humidity);

	Serial.print( F("airDataActive.pressure : "));
	Serial.println(airDataActive.pressure);

	Serial.print( F("lightDataActive.visible : "));
	Serial.println(lightDataActive.visible);

	Serial.print( F("lightDataActive.ir : "));
	Serial.println(lightDataActive.ir);

	Serial.print( F("lightDataActive.uv : "));
	Serial.println(lightDataActive.uv);
	
	Serial.print( F("vbatActive : "));
	Serial.println(vbatActive);

	Serial.print( F("soilMoitureActive : "));
	Serial.println(soilMoistureActive);

	Serial.println();
}


/**
*	CoolBoardSensors::setEnvSensorSetting():
*	This method is provided to set the enviornment
*	sensor settings , if argument is ommitted , default value will be assigned
*	
*/
void CoolBoardSensors::setEnvSensorSettings( uint8_t commInterface, uint8_t I2CAddress,    

						   uint8_t runMode , uint8_t tStandby, uint8_t filter, 						   
						   uint8_t tempOverSample,  uint8_t pressOverSample,    							   
						   uint8_t humidOverSample)
{

#if DEBUG == 1
	
	Serial.println( F("Entering CoolBoardSensors.setEnvSensorSettings()") );
	Serial.println();

#endif
  
	this->envSensor.settings.commInterface = commInterface;      

	this->envSensor.settings.I2CAddress = I2CAddress;

	this->envSensor.settings.runMode = runMode; 

	this->envSensor.settings.tStandby = tStandby; 

	this->envSensor.settings.filter = filter; 

	this->envSensor.settings.tempOverSample = tempOverSample;

	this->envSensor.settings.pressOverSample = pressOverSample;

	this->envSensor.settings.humidOverSample = humidOverSample;

}

/**
*	CoolBoardSensors::readVBat():
*	This method is provided to read the
*	Battery Voltage.
*
*	\return a float representing the battery
*	voltage
*/	
float CoolBoardSensors::readVBat()
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoardSensors.readVBat()") );
	Serial.println();

#endif

	digitalWrite(this->AnMplex, LOW);                            //Enable Analog Switch to get the batterie tension
  	
	delay(200);
  	
	int raw = analogRead(A0);                                    //read in batterie tension
 	
	float val = 6.04 / 1024 * raw;                               //convert it apprimatly right tension in volts
	
#if DEBUG == 1

	Serial.println( F("Vbat is : ") );
	Serial.println(val);
	Serial.println();

#endif

	return (val);	
}

/**
*	CoolBoardSensors::readMoisture():
*	This method is provided to red the
*	Soil Moisture
*
*	\return a float represnting the
*	soil moisture
*/
float CoolBoardSensors::readMoisture()
{

#if DEBUG == 1
	
	Serial.println( F("Entering CoolBoardSensors.readMoisture()") );
	Serial.println();
	
#endif

	digitalWrite(EnMoisture, LOW);                 //enable moisture sensor and waith a bit

	digitalWrite(AnMplex, HIGH);			//enable analog Switch to get the moisture

	delay(2000);

	int val = analogRead(A0);                       //read the value form the moisture sensor

	if (val >= 891){
		val = 890;
	}
	float result = (float)map(val, 0, 890, 0, 100);	

	digitalWrite(EnMoisture, HIGH);                  //disable moisture sensor for minimum wear
	
#if DEBUG == 1 

	Serial.println( F("RAW Moisture  is : "));
	Serial.println(val);
	Serial.println( F("Soil Moisture is : ") );
	Serial.println(result);
	Serial.println();

#endif 

	return (result);
}

