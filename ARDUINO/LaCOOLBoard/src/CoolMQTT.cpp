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
#include <ESP8266WiFi.h>
#include "CoolMQTT.h"
#include "ArduinoJson.h"
#include "CoolWifi.h"


#define DEBUG 0



/**
*	CoolMQTT::begin():
*	This method is provided to set the mqtt
*	client's parameters:	-client
*				-server
*				-callback method
*				-buffer size
*/
void CoolMQTT::begin()
{ 

#if DEBUG == 1 

	Serial.println( F("Entering CoolMQTT.begin()") );
	Serial.println();

#endif

	client.setClient(espClient);
	client.setServer(mqttServer, 1883);	
	client.setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });
	client.setBufferSize((unsigned short)bufferSize);

}

/**
*	CoolMQTT::state():
*	This method is provided to return the 
*	mqtt client's state.
*	\return mqtt client state:	
*		-4 : MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
*		-3 : MQTT_CONNECTION_LOST - the network connection was broken
*		-2 : MQTT_CONNECT_FAILED - the network connection failed
*		-1 : MQTT_DISCONNECTED - the client is disconnected cleanly
*		0 : MQTT_CONNECTED - the cient is connected
*		1 : MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
*		2 : MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
*		3 : MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
*		4 : MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
*		5 : MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
*/
int CoolMQTT::state()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolMQTT.state()") );
	Serial.println();	
	Serial.print( F("state : ") );
	Serial.println( this->client.state() );

#endif
	
	return( this->client.state() );
}

/**
*	CoolMQTT::connect( time to keep the connection alive in seconds ):
*	This method is provided to connect the client to the server,
*	publish to the out topic , subscribe to the in topic and set
*	the keepAlive time.
*	
*	\return mqtt client state
*/
int CoolMQTT::connect(unsigned long keepAlive)
{       

	int i=0;

#if DEBUG == 1 

	Serial.println( F("Entering CoolMQTT.connect()") );

#endif
	Serial.println( F("MQTT connecting...") );
	
	while( ( !this->client.connected() ) && ( i<5 ) ) 
	{
		// Attempt to connect
		if( this->client.connect( this-> user,keepAlive*10  ) )
		{
			client.subscribe( this->inTopic );

		#if DEBUG == 1 

			Serial.println( F(" subscribed , leavin ") ) ;
		
		#endif

			return( this->state() );
		}

		else
		{
		
		#if DEBUG == 1 

			Serial.println( F("not connected , retrying") );
		
		#endif

			
		}

	delay(5);
	i++;
	}
	if (state() == 0)
	{
		Serial.println( F("MQTT connected : OK") );
		Serial.println();
	}
	else Serial.println( F("MQTT not jet connected..."));
	return( this->state() );

}

/**
*	CoolMQTT::publish(data):
*	This method is provided to publish data
*	to the out topic. If the publish fails 
*   it deconnects and reconnects the Wifi 
*   and MQTT. This prevents lost MQTT syndrome
*   on cheaper Wifi hotspots
*
*	\return true if publish successful,
*	false otherwise
*/
bool CoolMQTT::publish(const char* data)
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolMQTT.publish()") );
	Serial.println();
	//data is in JSON, publish it directly

	Serial.println( F("data to publish : ") );
	Serial.println(data);
	Serial.print( F("data size : ") );
	Serial.println(strlen(data));

	Serial.println();

#endif
#if DEBUG == 0
	Serial.println( F("Publishing Message : "));
	Serial.println(data);
	Serial.println();
#endif

	bool pub=client.publish( this->outTopic,(byte*) data,strlen(data),false  );

#if DEBUG == 1 

	Serial.print( F("success : ") );
	Serial.println(pub);	

#endif
	if (pub == 1)
	{
		Serial.println( F("Publish : OK"));
	}
	else 
	{
		Serial.println( F("Publish : FAIL!!!"));
		Serial.print( F("Wifi State is : "));
		Serial.println(wifiManager.state());
		Serial.println( F("Deconnecting WiFi!"));
		wifiManager.disconnect();
		delay(500);
		Serial.println( F("Wifi State after deconnecting is : "));
		Serial.print(wifiManager.state());
		Serial.println( F("Re-connecting WiFi!"));
		wifiManager.begin();
		Serial.println( F("Wifi State after re-connecting is : "));
		Serial.print(wifiManager.state());
		begin();
	}
	return(pub);

}


/**
*	CoolMQTT::mqttLoop():
*	This method is provided to allow the
*	client to process the data
*
*	\return true if successful,false
*	otherwise
*/	
bool CoolMQTT::mqttLoop()
{

	unsigned long lastTime=millis();

#if DEBUG == 1

	Serial.println( F("Entering CoolMQTT.mqttLoop()") );
	Serial.println();

#endif	

	while( ( millis() - lastTime ) < 1000)
	{
		this->client.loop();
		yield();
	}

#if DEBUG == 1 
	
	Serial.print( F("loop result : ") );
	Serial.println( this->client.loop() );
	Serial.println();

#endif

	return( this->client.loop() );
}

/**
*	CoolMQTT::callback(in topic, incoming message , message length):
*	This method is provided to handle incoming messages from the
*	subscribed inTopic.
*	
*	Arguments are automatically assigned in client.setCallback()
*/
void CoolMQTT::callback(char* topic, byte* payload, unsigned int length) 
{

#if DEBUG == 1

	Serial.println( F("Entering CoolMQTT.callback() ") );
	Serial.println();

#endif 

	if(this->newMsg==false)
	{
		char temp[length+1];

	#if DEBUG == 1

		Serial.println( F("received temp msg : ") );
		
	#endif
		
		for (unsigned int i = 0; i < length; i++) 
		{
			temp[i]=(char)payload[i];
		
		#if DEBUG == 1 

			Serial.print( (char)payload[i] );
		
		#endif

		}
	
	#if DEBUG == 1 

		Serial.println();
		Serial.println( F("storing new message : ") );

		Serial.print(F("length : "));
		Serial.println(length);
		
		Serial.print(F("size : "));
		Serial.print(sizeof(payload));
		Serial.println();
	
	#endif

		this->newMsg=true;

		temp[length+1]='\0';

		this->msg=String(temp);
		this->msg.remove(length,1);
	
	#if DEBUG == 1 

		Serial.println( F("stored message : ") );
		Serial.println(this->msg);
	
	#endif

	}
	else
	{
	
	#if DEBUG == 1

		Serial.println( F("did not read last message") );
	
	#endif 
		
	}

}

/**
*	CoolMQTT::read():
*	This method is provided to return the last
*	read message.
*/
String CoolMQTT::read()
{	

#if DEBUG == 1 

	Serial.println( F("Entering CoolMQTT.read()") );
	Serial.println();

#endif 

	if(this->newMsg==true)
	{
		
		this->newMsg=false;

#if DEBUG == 1 
		Serial.println( F("received new message") );
		Serial.println( F("message : ") );
		Serial.println(this->msg);
		Serial.println();

#endif

		return(this->msg);
		
	}
	return("");

}

/**
*	CoolMQTT::config():
*	This method is provided to configure
*	the mqttClient :	-server
*				-inTopic
*				-outTopic
*				-client Id
*				-buffer size	
*
*	\return true if successful,false otherwise
*/
bool CoolMQTT::config()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolMQTT.config()") );
	Serial.println();

#endif

	//read config file
	//update data
	File configFile = SPIFFS.open("/mqttConfig.json", "r");

	if (!configFile) 
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to read /mqttConfig.json") );
		Serial.println();

	#endif

		return(false);
	}
	else
	{
		size_t size = configFile.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		configFile.readBytes(buf.get(), size);
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
		
			Serial.println( F("configuration json is ") );
			json.printTo(Serial);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();


		#endif

			if(json["mqttServer"].success() )
			{			
				const char* tempmqttServer = json["mqttServer"]; 
				for(int i =0;i< 50 ;i++)
				{
					mqttServer[i]=tempmqttServer[i];
				}
			}
			else
			{
				for(int i =0;i< 50 ;i++)
				{
					this->mqttServer[i]=this->mqttServer[i];
				}

			}
			json["mqttServer"]=this->mqttServer;

			
			if(json["inTopic"].success() )
			{
				const char* tempInTopic = json["inTopic"]; 
				for(int i =0;i< 50;i++)
				{
					inTopic[i]=tempInTopic[i];
				}
			}
			else
			{
				String tempMAC = WiFi.macAddress();
				tempMAC.replace(":","");
				snprintf(inTopic, 50, "$aws/things/%s/shadow/update/delta", tempMAC.c_str());	
			
			#if DEBUG == 1				
				
				Serial.print( F("Set Incomming MQTT Channel to : ") );
				Serial.println(inTopic);
			
			#endif	

			}
			json["inTopic"]=this->inTopic;
			
			
			if(json["outTopic"].success() )
			{
				const char* tempOutTopic = json["outTopic"]; 
				for(int i =0;i<50;i++)
				{
					outTopic[i]=tempOutTopic[i];
				}
			}
			else
			{
				String tempMAC = WiFi.macAddress();
				tempMAC.replace(":","");
				snprintf(outTopic, 50, "$aws/things/%s/shadow/update", tempMAC.c_str());
			
			#if DEBUG == 1 

				Serial.print( F("Set Outgoing MQTT Channel to : ") );
				Serial.println(outTopic);
			
			#endif

			}
			json["outTopic"]=this->outTopic;
		
			
			if(json["user"].success() )
			{				
				const char* tempUser = json["user"]; 
				for(int i =0;i<50;i++)
				{
					user[i]=tempUser[i];
				}
			}
			else
			{
				for(int i=0;i<50;i++)
				{
					this->user[i]=this->user[i];
				}				
			}
			json["user"]=this->user;
			
			if(json["bufferSize"].success() )
			{
				int tempBufferSize = json["bufferSize"]; 
				bufferSize=tempBufferSize;
			}
			else
			{
				this->bufferSize=this->bufferSize;
			}
			json["bufferSize"]=this->bufferSize;

			configFile.close();
			configFile = SPIFFS.open("/mqttConfig.json", "w");
			if(!configFile)
			{
			
			#if DEBUG == 1 

				Serial.println( F("failed to write to /mqttConfig.json") );
			
			#endif

				return(false);				
			}
			
			json.printTo(configFile);
			configFile.close();

		#if DEBUG == 1 

			Serial.println( F("saved configuration is :") );
			json.printTo(Serial);
			Serial.println();
		
		#endif

			return(true); 
		}
	}	
	

}

/**
*	CoolMQTT::config(server,in topic, out topic , user Id, buffer size):
*	This method is provided to manually configure the mqtt client	
*
*/
void CoolMQTT::config(const char mqttServer[],const char inTopic[],const char outTopic[],const char user[],int bufferSize)
{

#if DEBUG == 1

	Serial.println( F("Entering CoolMQTT.config() , no SPIFFS variant") );
	Serial.println();

#endif

	for(int i =0;i< 50 ;i++)
	{
		this->mqttServer[i]=mqttServer[i];
		this->inTopic[i]=inTopic[i];
		this->outTopic[i]=outTopic[i];
		this->user[i]=user[i];
	}
	this->bufferSize=bufferSize;
	

}

/**
*	CoolMQTT::printConf():
*	This method is provided to print the
*	configuration to the Serial Monitor
*/
void CoolMQTT::printConf()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolMQTT.printConf()") );
	Serial.println();	

#endif
	
	Serial.println("MQTT configuration ");

	Serial.print("mqttServer : ");
	Serial.println(this->mqttServer);

	Serial.print("inTopic : ");
	Serial.println(this->inTopic);

	Serial.print("outTopic : ");
	Serial.println(this->outTopic);

	Serial.print("user : ");
	Serial.println(this->user);

	Serial.print("bufferSize : ");
	Serial.println(this->bufferSize);

	Serial.println();


}

/**
*	CoolMQTT::getUser():
*	This method is provided to get the user name
*/
String CoolMQTT::getUser()
{

#if DEBUG == 1 
	Serial.println( F("Entering CoolMQTT.getUser()") );
	Serial.println();
	
	Serial.print( F("user : ") );
	Serial.println(this->user);

#endif

	return String(this->user);
}
