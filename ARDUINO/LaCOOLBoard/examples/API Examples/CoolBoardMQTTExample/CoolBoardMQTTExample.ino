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

/**
*	CoolBoardExample	
*
*	This example shows typical use
*	of the CoolBoard.
*		
*	Save this example in another location
*	in order to safely modify the configuration files
*	in the data folder.
*
*/

#include<CoolMQTT.h>
#include<CoolWifi.h>

CoolWifi wifiManager;
CoolMQTT mqtt;

String ssid[] = {"ssid1","ssid2"};//put up to 50 WiFi ssids
String pass[]={"pass1","pass2"};//put up to 50 WiFi passwords


const char mqttServer[]="----------";//the mqtt server you're going to use
const char inTopic[]="--------";//the topic you're going to subscribe to (receive from)
const char outTopic[]="-------";//the topic you're going to publish to (send to )
const char clientId[]="------";//your user name
int bufferSize = 128;//bytes
int keepAlive=15; //seconds


void setup()
{
	Serial.begin(115200);

	wifiManager.config(ssid,pass,2,180,0);
	wifiManager.begin();
	wifiManager.printConf();

	mqtt.config(mqttServer,inTopic, outTopic,clientId,bufferSize);
	mqtt.begin();
	mqtt.printConf();
	
	wifiManager.connect();

	mqtt.connect(keepAlive);
}

void loop()
{
mqtt.publish("hello world by CoolBoard");//publish this message to the outTopic
delay(1000);
Serial.println(mqtt.read());//print answer if answer is received
delay(1000);
mqtt.mqttLoop();
delay(1000);
}
