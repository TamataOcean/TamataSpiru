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


#ifndef CoolMQTT_H
#define CoolMQTT_H

#include"Arduino.h"  
#include <ESP8266WiFi.h>
#include "internals/CoolPubSubClient.h"
#include "CoolWifi.h"

/**
*	\class CoolMQTT
*	\brief This class handles the mqtt client
*/
class CoolMQTT
{

public:

	void begin();

	int connect(unsigned long keepAlive); //keepAlive in seconds

	bool publish(const char* data);

	bool publish(const char* data,unsigned long logInterval);

	String read();

	void config(const char mqttServer[],const char inTopic[],const char outTopic[],const char user[],int bufferSize);

	bool config();

	void callback(char* topic, byte* payload, unsigned int length);

	void printConf();

	int state();

	bool mqttLoop();

	String getUser();

private:
	
	/**
	*	MQTT Server name/ip
	*/
	char mqttServer[50]={'0'};

	/**
	*	String to store incoming messages
	*/
	String msg="";

	/**
	*	MQTT Topic to subscribe/listen to 
	*/
	char inTopic[50]={'0'};

	/**
	*	MQTT topic to publish/write to
	*/
	char outTopic[50]={'0'};

	/**
	*	MQTT user name
	*/
	char user[50]={'0'};
	
	/**
	*	MQTT inner bufferSize
	*/
	int bufferSize=3000;	
	
	/**
	*	WifiClient instance
	*/
	WiFiClient espClient;
	
	/**
	*	MQTT Client instance
	*/
	CoolPubSubClient client;

	/**
	*	WiFi Client instance
	*/
	CoolWifi wifiManager;

	/**
	*	new message flag
	*/
	bool newMsg=0;
	
};

#endif
