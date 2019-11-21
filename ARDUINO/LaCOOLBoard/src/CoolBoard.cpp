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
#include "CoolBoard.h"
#include "ArduinoJson.h"
#include "Arduino.h"
#include <Wire.h>
#include <memory>

#define DEBUG 1

/**
*	CoolBoard::CoolBoard():
*	This Constructor is provided to start
*	the I2C interface and Init the different
*	used pins
*/
CoolBoard::CoolBoard()
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoard Constructor") );
	Serial.println();

#endif
	
	Wire.begin(2, 14);                       //I2C init 

	pinMode(EnI2C, OUTPUT);		   //Declare I2C Enable pin 

}


/**
*	CoolBoard::begin():
*	This method is provided to configure and
*	start the used CoolKit Parts.
*	It also starts the first connection try
*	If Serial is enabled,it prints the configuration
*	of the used parts.
*/
void CoolBoard::begin()
{

#if DEBUG == 1

	Serial.println( F("Starting the CoolBoard  ")  );
	Serial.println( F("Entering CoolBoard.begin() ")  );
	Serial.println();
#endif	

#if DEBUG == 0
	Serial.println( F("Starting Coolboard..."));
#endif


	delay(100);
	
	coolBoardLed.write(255,128,0);//orange

	this->initReadI2C();
	delay(100);

	//read RTC config on startup and treat Off grid if the compile flag is set 
	rtc.config();
	rtc.offGrid();
	delay(200);

	coolBoardSensors.config();
	coolBoardSensors.begin();
	delay(200);

	onBoardActor.config();
	onBoardActor.begin();
	delay(100);
	
	wifiManager.config();
	wifiManager.begin();
	delay(100);

	mqtt.config();
	mqtt.begin();
	delay(100);

#if DEBUG == 1

	coolBoardLed.printConf();

	coolBoardSensors.printConf();

	onBoardActor.printConf();

	wifiManager.printConf();

	mqtt.printConf();
	

#endif


	if (jetpackActive)
	{
		jetPack.config();
		jetPack.begin();

	#if DEBUG == 1
		
		jetPack.printConf();

	#endif
		delay(100);
	}

	if (ireneActive)
	{
		irene3000.config();
		irene3000.startADC();
		coolBoardLed.write(128,128,128);
		delay(2000);
		int bValue=irene3000.readButton();
		if (bValue >= 65000) bValue = 8800;
		Serial.print(F("Irene Button : "));
		Serial.println(bValue);
		bValue=irene3000.readButton();
		if (bValue >= 65000) bValue = 8800;
		if (bValue < 2000)
		{
			Serial.println("Enter Irene config.");
			coolBoardLed.write(255,255,255);
			delay(5000);
			coolBoardLed.write(0,50,0);
			bValue=irene3000.readButton();
			if (bValue >= 65000) bValue = 0;
			while(bValue > 2000)
			{
				bValue=irene3000.readButton();
				Serial.println(irene3000.readButton());
				if (bValue >= 65000) bValue = 8800;
				delay(1000);
				yield();
				Serial.println(F("while 1"));
			}

			coolBoardLed.write(0,255,0);

			Serial.println(F("calibrating the Ph probe " ) );
			
			Serial.println(F("ph7 calibration for 25 seconds"));
			
			delay(10000);
			
			irene3000.calibratepH7();

			delay(15000);		
		
			irene3000.calibratepH7();

			coolBoardLed.write(50,0,0);
			bValue=irene3000.readButton();
			if (bValue >= 65000) bValue = 8800;
			while(bValue > 2000)
			{
				bValue=irene3000.readButton();
				Serial.println(irene3000.readButton());
				if (bValue >= 65000) bValue = 8800;
				delay(1000);
				yield();
				Serial.println(F("while 2"));
			}

			coolBoardLed.write(255,0,0);
			
			Serial.println(F("calibrating the Ph probe " ) );
			
			Serial.println(F("ph4 calibration for 25 seconds"));
			
			delay(10000);
			
			irene3000.calibratepH4();

			delay(15000);		
		
			irene3000.calibratepH4();

			irene3000.saveParams();

			coolBoardLed.write(50,0,50);
			bValue=irene3000.readButton();
			if (bValue >= 65000) bValue = 8800;
			while(bValue > 2000)
			{
				bValue=irene3000.readButton();
				Serial.println(irene3000.readButton());
				if (bValue >= 65000) bValue = 8800;
				delay(1000);
				yield();
				Serial.println(F("while 3"));
			}
		}
		



		/*irene3000.config();
		irene3000.begin();*/

	#if DEBUG == 1

		irene3000.printConf();

	#endif
		delay(100);
	}

	if (externalSensorsActive)
	{
		externalSensors.config();
		externalSensors.begin();

	#if DEBUG == 1

		externalSensors.printConf();

	#endif
		delay(100);
	}
	delay(100);
	coolBoardLed.fadeOut(255,128,0,0.5);//orange

	this->connect();

	delay(100);

	//RTC must be configured at startup to ensure RTC in Off Grid configuration

	rtc.begin();

	if( this->sleepActive==0 )	
	{	
		coolBoardLed.strobe(255,0,230,0.5);//shade of pink
		
		//send all config over mqtt

		this->sendConfig("CoolBoard","/coolBoardConfig.json");
	
		this->sendConfig("CoolSensorsBoard","/coolBoardSensorsConfig.json");

		this->sendConfig("CoolBoardActor","/coolBoardActorConfig.json");
	
		this->sendConfig("rtc","/rtcConfig.json");

		this->sendConfig("led","/coolBoardLedConfig.json");

		if(jetpackActive)
		{
			this->sendConfig("jetPack","/jetPackConfig.json");
		}

		if(ireneActive)
		{
			this->sendConfig("irene3000","/irene3000Config.json");
		}

		if(externalSensorsActive)
		{
			this->sendConfig("externalSensors","/externalSensorsConfig.json");
		}

		this->sendConfig("mqtt","/mqttConfig.json");
	}

#if DEBUG == 1

	rtc.printConf();

#endif
	delay(100);

	coolBoardLed.blink(0,255,0,0.5);//green

}

/**
*	CoolBoard::isConnected()
*	
*	This method is provided to check
*	if the card is connected to Wifi and MQTT
*
*	\return	 0 : connected
*		-1: Wifi Not Connected
*		-2: MQTT Not Connected
*			
*/
int CoolBoard::isConnected()
{

#if DEBUG == 1	

	Serial.println( F("Entering CoolBoard.isConnected ") );
	Serial.println();

#endif
	if (wifiManager.state() != WL_CONNECTED)
	{
	
		Serial.println(F("Wifi Not Connected"));

	#if DEBUG == 1

		Serial.println(F("Wifi State is "));
		Serial.println(wifiManager.state());
		
	#endif
		return(-1);
	}
	
	if(mqtt.state() != 0)
	{
		
		Serial.println( F("MQTT not Connected"));

	#if DEBUG==1
		Serial.println( F("mqtt state is :") );
		Serial.println(mqtt.state());	
	
	#endif

	}
	
	return(0);

}


/**
*	CoolBoard::connect():
*	This method is provided to manage the network
*	connection and the mqtt connection.
*
*	 \return mqtt client state
*/
int CoolBoard::connect()
{

#if DEBUG == 1	

	Serial.println( F("Entering CoolBoard.connect ") );
	Serial.println();
	Serial.println( F("Connecting the CoolBoard  ") );
	delay(100);

#endif
	//coolBoardLed.write(0,0,255);//blue
	delay(10);
	coolBoardLed.write(0,0,255);//blue

	
			
	
#if DEBUG == 1		

	Serial.println( F("Launching CoolWifi") );
	Serial.println();

#endif
	wifiManager.connect();
	delay(100);


	//only attempt MQTT connection when Wifi is Connected
	if (wifiManager.state() == WL_CONNECTED)
	{

	#if DEBUG == 1	
	
		Serial.println( F("Launching mqtt.connect()") );
		Serial.println();
	
	#endif	
		//logInterval in seconds
		mqtt.connect(this -> getLogInterval()*2);
		delay(100);
		if (mqtt.state() <0 && wifiManager.nomad == 1)
		{
			Serial.println( F("Known WIFI in the area but no internet connection"));
			Serial.println( F("  --->   Launching Configuration Portal   <---"));
			wifiManager.disconnect();
			delay(200);
			coolBoardLed.write(255,128,255);//whiteish violet..
			wifiManager.connectAP();
		}	
	}
	
	
	
	
#if DEBUG == 1

	Serial.println( F("mqtt state is :") );
	Serial.println(mqtt.state());
	Serial.println();
	delay(100);

#endif

	coolBoardLed.blink(0,0,255,0.5);//blue

	return(mqtt.state());
}

/**
*	CoolBoard::onLineMode():
*	This method is provided to manage the online
*	mode:	-update clock
*		-read sensor
*		-do actions
*		-publish data
*		-read answer
*		-update config
*/
void CoolBoard::onLineMode()
{

	coolBoardLed.fadeIn(128,255,50,0.5);//shade of green

#if DEBUG == 1

	Serial.println( F("Entering CoolBoard.onLineMode() ") );
	Serial.println();

#endif
#if DEBUG == 0

	Serial.println( F("CoolBoard is in Online Mode"));

#endif

	//check if we hit mqtt Timeout
	if (mqtt.state() != 0)
	{
		Serial.println( F("reconnecting MQTT..."));

		mqtt.connect(this -> getLogInterval()*2);
		delay(200);
	}

	data="";
	answer="";

	//send saved data if any
	if(fileSystem.isDataSaved())
	{

		coolBoardLed.fadeIn(128,128,255,0.5);//shade of blue

		Serial.println( F("There is data saved on the File System") );
		Serial.println( F("Sending saved data over MQTT ") );
		Serial.println();
		coolBoardLed.strobe(128,128,255,0.5);//shade of blue 

		mqtt.publish("sending saved data");
		mqtt.mqttLoop();


		
		int size=0;
		std::unique_ptr<String[]> savedData(std::move(fileSystem.getSensorSavedData(size)));//{..,..,..}

		int i=0;
		//loop through the array
		while(i<size)
		{
			//formatting data:
		
			String jsonData = "{\"state\":{\"reported\":";
			jsonData += savedData[i]; // {"state":{"reported":{..,..,..,..,..,..,..,..}
			jsonData += " } }"; // {"state":{"reported":{..,..,..,..,..,..,..,..}  } }

		#if DEBUG == 1 
			Serial.println(F("Size is : "));
			Serial.println(size);
			Serial.print(F("sending line N°"));
			Serial.println(i);
			Serial.println(jsonData);
			Serial.println();

		#endif

			coolBoardLed.strobe(128,128,255,0.5);//shade of blue
		
			mqtt.publish( jsonData.c_str() );
			mqtt.mqttLoop();
		
			coolBoardLed.fadeOut(128,128,255,0.5);//shade of blue
			
			i++;
			yield();
		}		


	#if DEBUG == 1

		Serial.println( F("Saved data sent ") );
		Serial.println();
	
	#endif

	}

	coolBoardLed.blink(128,255,50,0.5);//shade of green

	//clock update
	Serial.println( F("Re-checking RTC..."));
	rtc.update();

	//read user data if user is active
	if(userActive)
	{
		coolBoardLed.fadeIn(245,237,27,0.5);//shade of yellow
	
	#if DEBUG == 1

		Serial.println( F("User is Active") );
		Serial.println( F("Collecting User's data ( mac,username,timeStamp )") );
		Serial.println();
	
	#endif	
		coolBoardLed.blink(245,237,27,0.5);//shade of yellow	

		//reading user data
		data=this->userData();//{"":"","":"","",""}

		//formatting json 
		data.setCharAt( data.lastIndexOf('}') , ',');//{"":"","":"","","",
				
		//read sensors data
	#if DEBUG == 1

		Serial.println( F("Collecting sensors data ") );
		Serial.println();
	
	#endif

		data+=this->readSensors();//{"":"","":"","","",{.......}		

		//formatting json correctly
		data.remove(data.lastIndexOf('{'), 1);//{"":"","":"","","",.......}
		
		coolBoardLed.fadeOut(245,237,27,0.5);//shade of yellow
				
	}	
	else
	{
		//read sensors data
	#if DEBUG == 1

		Serial.println( F("Collecting sensors data ") );
		Serial.println();
	
	#endif
		coolBoardLed.fade(190,100,150,0.5);//shade of violet		
		data=this->readSensors();//{..,..,..}
	}


	//do action
	if(this->manual == 0 )
	{
		//get Time for temporal actors
		tmElements_t tm;
		tm=rtc.getTimeDate();

		if (jetpackActive)
		{
			#if DEBUG ==1
			Serial.println( F("jetpack is Active ") );
			Serial.println();
			#endif

			Serial.println( F("jetpack doing action ") );
			coolBoardLed.fade(100,100,150,0.5);//dark shade of blue		
			byte lastAction = jetPack.action;//keeps last value to see if state changes

			String jetpackStatus= jetPack.doAction(data.c_str(), int(tm.Hour), int(tm.Minute));//{....}
			
			if(lastAction != jetPack.action)//send a message if a actor has changed 
			{
				data+=jetpackStatus;//{..,..,..}{..,..,..}
				data.remove(data.lastIndexOf('{'), 1);//{..,..,..}..,..,..}
				data.setCharAt( data.indexOf('}') , ',');//{..,..,..,..,..,..}

				//sending jetpackStatus over MQTT
				String jsonJetpackStatus="{\"state\":{\"reported\":";
				jsonJetpackStatus+=jetpackStatus;
				jsonJetpackStatus += " } }"; // {"state":{"reported":{..,..,..,..,..,..,..,..}  } }
				mqtt.publish( jsonJetpackStatus.c_str() );			
			}
		}
		bool lastActionB = digitalRead(onBoardActor.pin);
		String onBoardActorStatus=onBoardActor.doAction( data.c_str(), int(tm.Hour), int(tm.Minute) );	
		if(lastActionB != digitalRead(onBoardActor.pin))//send a message if actor has changed
		{
			data+=onBoardActorStatus;//{..,..,..}{..,..,..}
			data.remove(data.lastIndexOf('{'), 1);//{..,..,..}..,..,..}
			data.setCharAt( data.indexOf('}') , ',');//{..,..,..,..,..,..}

			//sending onBoardActorStatus over MQTT
			String jsonOnBoardActorStatus="{\"state\":{\"reported\":";
			jsonOnBoardActorStatus+=onBoardActorStatus;
			jsonOnBoardActorStatus += " } }"; // {"state":{"reported":{..,..,..,..,..,..,..,..}  } }
			mqtt.publish( jsonOnBoardActorStatus.c_str() );
		}
	}
	else if(this->manual == 1 )
	{
		Serial.println(F("we are in manual mode"));
		mqtt.mqttLoop();
		answer = mqtt.read();
		this -> update(answer.c_str());
	}

	delay(50);
	coolBoardLed.fadeIn(128,255,50,0.5);//shade of green

	//formatting data:
	String jsonData = "{\"state\":{\"reported\":";
	jsonData += data; // {"state":{"reported":{..,..,..,..,..,..,..,..}
	jsonData += " } }"; // {"state":{"reported":{..,..,..,..,..,..,..,..}  } }
	
	//mqtt client loop to allow data handling
	mqtt.mqttLoop();

	coolBoardLed.blink(128,255,50,0.5);//shade of green	

	//read mqtt answer
	answer = mqtt.read();

#if DEBUG == 1 

	Serial.println( F("checking if there's an MQTT message ")  );
	Serial.println( F("answer is : ") );	
	Serial.println(answer);	
	Serial.println();

#endif	

	coolBoardLed.fadeOut(128,255,50,0.5);//shade of green	

	//check if the configuration needs update 
	//and update it if needed 
	this -> update(answer.c_str());
	
	coolBoardLed.fadeIn(128,255,50,0.5);//shade of green	
	
	//check if we hit mqtt Timeout
	if (mqtt.state() != 0)
	{
		Serial.println( F("reconnecting MQTT..."));

		mqtt.connect(this -> getLogInterval()*2);
		delay(200);
	}

	//log interval is passed in seconds, logInterval*1000 = logInterval in ms, if no log was sent till now (so it also works when sleep is active since previusLogTime is 0 on startup
	if( ( millis() - ( this->previousLogTime)  ) >= ( logInterval*1000 ) || (this->previousLogTime == 0) )
	{
		//publishing data	
		if( this->sleepActive==0 )	
		{	
			coolBoardLed.strobe(255,0,230,0.5);//shade of pink
			
			//logInterval in seconds
			mqtt.publish( jsonData.c_str());//, this->getLogInterval() );

			mqtt.mqttLoop();
		
		}
		else
		{
			coolBoardLed.strobe(230,255,0,0.5);//shade of yellow	

			mqtt.publish(jsonData.c_str());	
		
			mqtt.mqttLoop();

			answer = mqtt.read();

			this ->update(answer.c_str());

			//logInterval in seconds
			this->sleep( this->getLogInterval() ) ;
		}
		this->previousLogTime=millis();
	}

	coolBoardLed.fadeOut(128,255,50,0.5);//shade of green		

	mqtt.mqttLoop();

	//read mqtt answer
	answer = mqtt.read();
	this -> update(answer.c_str());	
	coolBoardLed.blink(128,255,50,0.5);//shade of green	


}


/**
*	CoolBoard::offlineMode():
*	This method is provided to manage the offLine
*	mode:	-read sensors
*		-do actions
*		-save data in the file system
*		-if there is WiFi but no Internet : make data available over AP
*		-if there is no connection : retry to connect
*/
void CoolBoard::offLineMode()
{
	coolBoardLed.fade(51,100,50,0.5);//dark shade of green	
#if DEBUG == 1	
	
	Serial.println( F("Entering off line mode ") );	
	
#endif

#if DEBUG == 0

	Serial.println( F("CoolBoard is in Offline Mode"));

#endif

	//read user data if user is active
	if(userActive)
	{

		coolBoardLed.fadeIn(245,237,27,0.5);//shade of yellow

	#if DEBUG == 1
		
		Serial.println( F("User is Active") );
		Serial.println( F("Collecting User's data ( mac,username,timeStamp )") );
		Serial.println();

	#endif

		coolBoardLed.blink(245,237,27,0.5);//shade of yellow	

		//reading user data
		data=this->userData();//{"":"","":"","",""}

		//formatting json 
		data.setCharAt( data.lastIndexOf('}') , ',');//{"":"","":"","","",
		
				
		//read sensors data

		Serial.println( F("Collecting sensors data ") );
		Serial.println();

		data+=this->readSensors();//{"":"","":"","","",{.......}

		

		//formatting json correctly
		data.remove(data.lastIndexOf('{'), 1);//{"":"","":"","","",.......}

		coolBoardLed.fadeOut(245,237,27,0.5);//shade of yellow
				
	}	
	else
	{
		//read sensors data
	#if DEBUG == 1

		Serial.println( F("Collecting sensors data ") );
		Serial.println();

	#endif

		coolBoardLed.fade(190,100,150,0.5);//shade of violet		

		data=this->readSensors();//{..,..,..}
	}

	coolBoardLed.fade(51,100,50,0.5);//dark shade of green	

	//get Time for temporal actors
	tmElements_t tm;
	tm=rtc.getTimeDate();

	//do action

	if (jetpackActive)
	{
	


	#if DEBUG == 1

		Serial.println( F("jetpack is Active ") );
		Serial.println( F("jetpack doing action ") );
		Serial.println();
	
	#endif
		coolBoardLed.fade(100,100,150,0.5);//dark shade of blue	
	
		data+=jetPack.doAction(data.c_str(), int(tm.Hour), (tm.Minute));//{..,..,..}{..,..,..}
			
		data.remove(data.lastIndexOf('{'), 1);//{..,..,..}..,..,..}
			
		data.setCharAt( data.indexOf('}') , ',');//{..,..,..,..,..,..}


	}
	
	delay(50);

	if (onBoardActor.actor.actif == true)
	{
		data+=onBoardActor.doAction( data.c_str(), int(tm.Hour), int(tm.Minute)  );//{..,..,..}{..,..,..}

		data.remove(data.lastIndexOf('{'), 1);//{..,..,..}..,..,..}
			
		data.setCharAt( data.indexOf('}') , ',');//{..,..,..,..,..,..}
	}

	coolBoardLed.fade(51,100,50,0.5);//dark shade of green	
	//log interval is passed in seconds, logInterval*1000 = logInterval in ms, if no log was sent till now (so it also works when sleep is active since previusLogTime is 0 on startup
	if( ( millis() - ( this->previousLogTime)  ) >= ( logInterval*1000 ) || (this->previousLogTime == 0) )
	{
		//saving data in the file system as JSON
		if (this->saveAsJSON == 1)
		{
			fileSystem.saveSensorData( data.c_str() );
			Serial.println( F("saving Data as JSON in Memory : OK"));
		}

		if (this->saveAsCSV == 1)
		{
			fileSystem.saveSensorDataCSV( data.c_str() );
			Serial.println( F("saving Data as CSV in Memory : OK"));
		}

		this->previousLogTime=millis();
	}

	coolBoardLed.fadeOut(51,100,50,0.5);//dark shade of green
	if (wifiManager.nomad == 0 || this->sleepActive == 0)
	{
		//case we have wifi but no internet
		if( (wifiManager.state() == WL_CONNECTED) && ( mqtt.state()!=0 ) )
		{
			
			Serial.println(F("there is Wifi but no Internet"));
			Serial.println(F("lunching AP to check saved files"));
			Serial.println(F("and Add new WiFi if needed"));
			
			wifiManager.connectAP();
			
		}

		//case we have no connection at all
		if( wifiManager.state() != WL_CONNECTED )
		{

		#if DEBUG == 1
			
			Serial.println(F("there is No Wifi 1"));
			Serial.println(F("retrying to connect"));

		#endif

		#if DEBUG == 0
			Serial.println( F("there is no WiFi..."));
		#endif
			
			this->connect();//nomad case : just run wifiMulti
					//normal case : run wifiMulti+AP
			
		}	
	}

	if( this->sleepActive==1 )	
	{
		this->sleep( this->getLogInterval() ) ;
	}

}

/**
*	CoolBoard::config():
*	This method is provided to configure
*	the CoolBoard :	-log interval
*			-irene3000 activated/deactivated
*			-jetpack activated/deactivated
*			-external Sensors activated/deactivated
*			-mqtt server timeout
*
*	\return true if configuration is done,
*	false otherwise
*/
bool CoolBoard::config()
{
	yield();

	Serial.println();
	Serial.println("[" + WiFi.macAddress() + "]");
#if DEBUG == 1

	Serial.println( F("Entering CoolBoard.config() ") );
	Serial.println();

#endif
#if DEBUG == 0
	Serial.println();
	Serial.println( F("Loading configuration for this CoolBoard..."));
#endif 

	//open file system
	fileSystem.begin();
	
	//start the led
	coolBoardLed.config();
	coolBoardLed.begin();
	delay(10);
	coolBoardLed.blink(243,171,46,0.5);//shade of orange		

	
	//open configuration file
	File configFile = SPIFFS.open("/coolBoardConfig.json", "r");
	
	if (!configFile)

	{
	
		Serial.println( F("failed to read /coolBoardConfig.json  ") );

		coolBoardLed.blink(255,0,0,0.5);//shade of red		
		return(false);
	}

	else
	{
		size_t size = configFile.size();

		// Allocate a buffer to store contents of the file.
		std::unique_ptr < char[] > buf(new char[size]);

		configFile.readBytes(buf.get(), size);

		DynamicJsonBuffer jsonBuffer;

		JsonObject & json = jsonBuffer.parseObject(buf.get());

		if (!json.success())
		{
		
			Serial.println( F("failed to parse CoolBoard Config json object ") );
	
			coolBoardLed.blink(255,0,0,0.5);//shade of red		
			return(false);
		}

		else
		{	
		
		#if DEBUG == 1
			
			Serial.println( F("configuration json : ") );
			json.printTo(Serial);
			Serial.println();
			
			Serial.print(F("jsonBuffer size : "));
			Serial.print(jsonBuffer.size());
			Serial.println();

		#endif
			
			//parsing userActive Key
			if (json["userActive"].success())
			{
				this -> userActive = json["userActive"];
			}

			else
			{
				this -> userActive = this -> userActive;
			}
			json["userActive"] = this -> userActive;

			//parsing logInterval key
			if (json["logInterval"].success())
			{
				this -> logInterval = json["logInterval"].as<unsigned long>();
			}
			else
			{
				this -> logInterval = this -> logInterval;
			}
			json["logInterval"] = this -> logInterval;
			
			//parsing ireneActive key			
			if (json["ireneActive"].success())
			{
				this -> ireneActive = json["ireneActive"];
			}
			else
			{
				this -> ireneActive = this -> ireneActive;
			}
			json["ireneActive"] = this -> ireneActive;
			
			//parsing jetpackActive key
			if (json["jetpackActive"].success())
			{
				this -> jetpackActive = json["jetpackActive"];
			}
			else
			{
				this -> jetpackActive = this -> jetpackActive;
			}
			json["jetpackActive"] = this -> jetpackActive;

			//parsing externalSensorsActive key
			if (json["externalSensorsActive"].success())
			{
				this -> externalSensorsActive = json["externalSensorsActive"];
			}
			else
			{
				this -> externalSensorsActive = this -> externalSensorsActive;
			}
			json["externalSensorsActive"] = this -> externalSensorsActive;

			
			//parsing sleepActive key
			if (json["sleepActive"].success())
			{
				this -> sleepActive = json["sleepActive"];
			}
			else
			{
				this -> sleepActive = this -> sleepActive;
			}
			json["sleepActive"] = this -> sleepActive;


			//parsing manual key
			if (json["manual"].success())
			{
				this -> manual = json["manual"].as<bool>();
			}
			else
			{
				this -> manual = this -> manual;
			}
			json["manual"] = this -> manual;

			//parsing saveAsJSON key
			if (json["saveAsJSON"].success())
			{
				this -> saveAsJSON = json["saveAsJSON"].as<bool>();
			}
			else
			{
				this -> saveAsJSON = this -> saveAsJSON;
			}
			json["saveAsJSON"] = this -> saveAsJSON;

			//parsing saveAsCSV key
			if (json["saveAsCSV"].success())
			{
				this -> saveAsCSV = json["saveAsCSV"].as<bool>();
			}
			else
			{
				this -> saveAsCSV = this -> saveAsCSV;
			}
			json["saveAsCSV"] = this -> saveAsCSV;

			//saving the current/correct configuration
			configFile.close();
			configFile = SPIFFS.open("/coolBoardConfig.json", "w");
			if (!configFile)
			{
			
				Serial.println( F("failed to write to /coolBoardConfig.json") );
				Serial.println();

 				coolBoardLed.blink(255,0,0,0.5);//shade of red		
				return(false);
			}

			json.printTo(configFile);
			configFile.close();
			coolBoardLed.blink(0,255,0,0.5);//green blink

			#if DEBUG == 0

				Serial.println( F("Configuration loaded : OK"));
				Serial.println();

			#endif

			return(true);
		}
	}

	//coolBoardLed.strobe(243,171,46,0.5);//shade of orange
	
	coolBoardLed.fadeOut(243,171,46,0.5);//shade of orange				
}

/**
*
*	CoolBoard::printConf():
*	This method is provided to print
*	the configuration to the Serial
*	Monitor.
*/
void CoolBoard::printConf()
{

#if DEBUG == 1
	
	Serial.println( F("Entering CoolBoard.printConf() ") );
	Serial.println();

#endif

	Serial.println( F("Printing Cool Board Configuration "));
	Serial.print( F("log interval 		: "));
	Serial.println(this->logInterval);

	Serial.print( F("irene active 		: "));
	Serial.println(this->ireneActive);

	Serial.print( F("jetpack active		: "));
	Serial.println(this->jetpackActive);

	Serial.print( F("external sensors active 	: "));
	Serial.println(this->externalSensorsActive);

	Serial.print( F("sleep active 		: "));
	Serial.println(this->sleepActive);

	Serial.print( F("user active 		: "));
	Serial.println(this->userActive);

	Serial.print( F("manual active		: "));
	Serial.println(this->manual);

	Serial.print( F("saveAsJSON active		: "));
	Serial.println(this->saveAsJSON);

	Serial.print( F("saveAsCSV active		: "));
	Serial.println(this->saveAsCSV);

	Serial.println();




}

/**
*	CoolBoard::update(mqtt answer):
*	This method is provided to handle the
*	configuration update of the different parts
*/
void CoolBoard::update(const char * answer)
{
	coolBoardLed.fadeIn(153,76,0,0.5);//shade of brown		

	#if DEBUG == 1
	Serial.println( F("Entering CoolBoard.update() ") );
	Serial.println();
	Serial.println( F("message is : ") );
	Serial.println(answer);
	Serial.println();
	#endif

	DynamicJsonBuffer jsonBuffer;
	JsonObject & root = jsonBuffer.parseObject(answer);
	JsonObject & stateDesired = root["state"];

	#if DEBUG == 1
	Serial.println( F("root json : ") );
	root.printTo(Serial);
	Serial.println();
	Serial.println( F("stateDesired json : "));
	stateDesired.printTo(Serial);
	Serial.println();
	Serial.print( F("jsonBuffer size : "));
	Serial.println(jsonBuffer.size());
	#endif
	
	if (stateDesired["CoolBoard"]["manual"].success())
	{
		this -> manual = stateDesired["CoolBoard"]["manual"].as<bool>();
	
		#if DEBUG == 1
		Serial.println("Manual Flag received");
		Serial.println(this -> manual);
		#endif
	}
	if (stateDesired.success())
	{
		#if DEBUG == 1
		Serial.println( F("update message parsing : success") );
		Serial.println();
		#endif

		String answerDesired;	
		stateDesired.printTo(answerDesired);
		
		#if DEBUG == 1		
		Serial.println( F("update is ok ") );
		Serial.println( F("desired update is : ") );			
		Serial.println(answerDesired);
		Serial.println("json size is : ");
		Serial.println(jsonBuffer.size() ) ;				
		Serial.println();
		Serial.print("ManualFlag : ");
		Serial.println(this->manual);
		#endif

		//manual mode check
		if(this->manual == 1 )
		{ 
			Serial.println();
			Serial.print("Enter Manual Actors...");
			//json parse for actors
			for(auto kv : stateDesired)
			{
				#if DEBUG == 1
				Serial.print(F("writing to "));
				Serial.println(kv.key);
				Serial.print(F("state : "));
				Serial.println(kv.value.as<bool>());		
				#endif				

				if( strcmp(kv.key,"Act0") == 0 )
				{
					jetPack.writeBit(0,kv.value.as<bool>() ); 	
				}
				else if(strcmp(kv.key,"Act1") == 0)
				{
					jetPack.writeBit(1,kv.value.as<bool>() ); 
				}
				else if(strcmp(kv.key,"Act2") == 0)
				{
					jetPack.writeBit(2,kv.value.as<bool>() ); 
				}
				else if(strcmp(kv.key,"Act3") == 0)
				{
					jetPack.writeBit(3,kv.value.as<bool>() ); 
				}
				else if(strcmp(kv.key,"Act4") == 0)
				{
					jetPack.writeBit(4,kv.value.as<bool>() ); 
				}
				else if(strcmp(kv.key,"Act5") == 0)
				{
					jetPack.writeBit(5,kv.value.as<bool>() ); 
				}
				else if(strcmp(kv.key,"Act6") == 0)
				{
					jetPack.writeBit(6,kv.value.as<bool>() ); 
				}
				else if (strcmp(kv.key,"Act7") == 0)
				{
					jetPack.writeBit(7,kv.value.as<bool>() ); 
				}
				else if (strcmp(kv.key,"ActB") == 0)
				{
					onBoardActor.write(kv.value.as<bool>() ); 
				}
			}
		}

		//Irene calibration through update message
		if(stateDesired["calibration"].success())
		{
			Serial.println(F("Starting Irene Calibration From MQTT Update"));
			Serial.println();	
			delay(2000);
			Serial.println(F("ph7 calibration for 25 seconds"));
			delay(10000);
			irene3000.calibratepH7();
			delay(15000);		
			irene3000.calibratepH7();
			delay(1000);
			Serial.println(F("ph 7 calibration ok"));
			Serial.println();
			Serial.println(F("ph 4 calibration for 25 seconds"));
			delay(10000);		
			irene3000.calibratepH4();
			delay(15000);
			irene3000.calibratepH4();
			delay(1000);		
			Serial.println(F("ph 4 calibration ok"));
			Serial.println();
			irene3000.saveParams();
		}

		//saving the new configuration
		fileSystem.updateConfigFiles(answerDesired);
	    //answering the update msg:
		//reported = received configuration
		//desired=null
	
		String updateAnswer;
		String tempString;
		
		stateDesired.printTo(tempString);
		updateAnswer="{\"state\":{\"reported\":";
		updateAnswer+=tempString;
		updateAnswer+=",\"desired\":null}}";

		#if DEBUG == 1
		Serial.println( F("preparing answer message ") );
		Serial.println();
		Serial.println( F("updateAnswer : ") );
		Serial.println(updateAnswer);
		#endif	

		mqtt.publish(updateAnswer.c_str());
		mqtt.mqttLoop();
		delay(10);
		if(manual == 0 )//&& !dedicatedTransmission )
		{
			//restart the esp to apply the config
			ESP.restart();
		}
	}
	else
	{
		#if DEBUG == 1
		Serial.println( F("Failed to parse update message( OR no message received )") );
		Serial.println();
		#endif
	}

	coolBoardLed.strobe(153,76,0,0.5);//shade of brown
	coolBoardLed.fadeOut(153,76,0,0.5);//shade of brown								
}

/**
*	CoolBoard::getLogInterval():
*	This method is provided to get
*	the log interval
*
*	\return interval value in s
*/
unsigned long CoolBoard::getLogInterval()
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoard.getLogInterval() ") );
	Serial.println();
	Serial.println( F("log Interval is :") );
	Serial.println(logInterval);
	Serial.println();

#endif

	return(this -> logInterval);
}

/**
*	CoolBoard::readSensors():
*	This method is provided to read and
*	format all the sensors data in a
*	single json.
*	
*	\return	json string of all the sensors read.
*/
String CoolBoard::readSensors()
{

	coolBoardLed.fadeIn(128,255,0,0.5);//light shade of green
				
#if DEBUG == 1

	Serial.println( F("Entering CoolBoard.readSensors()") );
	Serial.println();

#endif
	coolBoardLed.strobe(128,255,0,0.5);//light shade of green

	String sensorsData;
	
	this->initReadI2C();

	sensorsData = coolBoardSensors.read(); // {..,..,..}
	
	if (externalSensorsActive)
	{
		sensorsData += externalSensors.read(); // {..,..,..}{..,..}
		sensorsData.setCharAt(sensorsData.lastIndexOf('}'), ','); // {..,..,..}{..,..,
		sensorsData.setCharAt(sensorsData.lastIndexOf('{'), ','); // {..,..,..},..,..,
		sensorsData.remove(sensorsData.lastIndexOf('}'), 1); // {..,..,..,..,..,
		sensorsData.setCharAt(sensorsData.lastIndexOf(','), '}'); // {..,..,..,..,..}

	}
	if (ireneActive)
	{
		sensorsData += irene3000.read(); // {..,..,..,..,..}{..,..,..}

		sensorsData.setCharAt(sensorsData.lastIndexOf('}'), ','); // {..,..,..,..,..}{..,..,..,
		sensorsData.setCharAt(sensorsData.lastIndexOf('{'), ','); // {..,..,..,..,..},..,..,..,
		sensorsData.remove(sensorsData.lastIndexOf('}'), 1); // {..,..,..,..,..,..,..,..,
		sensorsData.setCharAt(sensorsData.lastIndexOf(','), '}'); // {..,..,..,..,..,..,..,..}		
		
		
	}

	//getting Hour:
	/*tmElements_t tm;
	tm=rtc.getTimeDate();
	
	//adding Hour
	sensorsData.remove(sensorsData.lastIndexOf('}'), 1); // {..,..,..,..,..,..,..,..,	
	sensorsData+=",\"hour\":";	
	sensorsData+=tm.Hour;
	sensorsData+=",\"minute\":";
	sensorsData+=tm.Minute;
	sensorsData+="}";*/
	
#if DEBUG == 1
	Serial.println();
	Serial.println( F("sensors data is ") );
	Serial.println(sensorsData);
	Serial.println();

#endif
	coolBoardLed.fadeOut(128,255,0,0.5);//light shade of green

	return(sensorsData);

}

/**
*	CoolBoard::initReadI2C():
*	This method is provided to enable the I2C
*	Interface.
*/
void CoolBoard::initReadI2C()
{

#if DEBUG == 1

 	Serial.println( F("Entering CoolBoard.initReadI2C()") );
	Serial.println();

#endif
 
	digitalWrite(EnI2C,HIGH);//HIGH= I2C Enable

}


/**
*	CoolBoard::userData():
*	This method is provided to
*	return the user's data.
*	
*	\return json string of the user's data
*/
String CoolBoard::userData()
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoard.userData() ") );
	Serial.println();

#endif

	String tempMAC = WiFi.macAddress();

	tempMAC.replace(":", "");

	String userJson = "{\"user\":\"";

	userJson += mqtt.getUser();

	userJson += "\",\"timestamp\":\"";

	userJson += rtc.getESDate(); // "timestamp":"20yy-mm-ddThh:mm:ssZ"

	userJson += "\",\"mac\":\"";

	userJson += tempMAC;

	userJson += "\"}";

#if DEBUG == 1

	Serial.println( F("userData is : ") );
	Serial.println(userJson);
	Serial.println();

#endif	
	
	return(userJson);
	
}


/**
*	CoolBoard::sleep(int interval):
*	This method is provided to allow the
*	board to enter deepSleep mode for
*	a period of time equal to interval in s 
*/
void CoolBoard::sleep(unsigned long interval)
{

	Serial.println( F("Entering CoolBoard.sleep() ") );
	Serial.print( F("going to sleep for ") );
	Serial.print(interval);
	Serial.println(F("s") );
	Serial.println();
	
	//interval is in seconds , interval*1000*1000 in µS
	ESP.deepSleep ( ( interval * 1000 * 1000 ), WAKE_RF_DEFAULT) ;

}


/**
*	CoolBoard::sendConfig( module Name,file path ):
*	This method is provided to send	
*	all the configuration files
*	over MQTT
*
*	\return true if successful, false if not
*/
bool CoolBoard::sendConfig(const char* moduleName, const char* filePath)
{

#if DEBUG == 1 

	Serial.println(F("Entering CoolBoard.sendConfig()"));

#endif

	String result;

	//open file
	File configFile = SPIFFS.open(filePath, "r");
	
	if (!configFile)

	{
	
		Serial.print( F("failed to read ") );

		Serial.println(filePath);

		return(false);
	}
	else
	{

		//file to json
		size_t size = configFile.size();

		// Allocate a buffer to store contents of the file.
		std::unique_ptr < char[] > buf(new char[size]);

		configFile.readBytes(buf.get(), size);

		DynamicJsonBuffer jsonBuffer;

		JsonObject & json = jsonBuffer.parseObject(buf.get());

		if (!json.success())
		{
		
			Serial.println( F("failed to parse json object ") );

			return(false);
		}

		else
		{	
		
		#if DEBUG == 1
			
			Serial.println( F("configuration json : ") );
			json.printTo(Serial);
			Serial.println();
			
			Serial.print(F("jsonBuffer size : "));
			Serial.print(jsonBuffer.size());
			Serial.println();

		#endif

			//json to string
			String temporary;

			json.printTo(temporary);
			
			//format string
			result = "{\"state\":{\"reported\":{\"";
			result += moduleName; //{"state":{"reported":{"moduleName
			result += "\":";//{"state":{"reported":{"moduleName":
			result += temporary;//{"state":{"reported":{"moduleName":{..,..,..,..,..,..,..,..}
			result += "} } }"; // {"state":{"reported":{..,..,..,..,..,..,..,..}  } } }
			
			//string over mqtt
			mqtt.publish(result.c_str());
			
			mqtt.mqttLoop();
			
			return(true);
			
		}
	
	}	
}


