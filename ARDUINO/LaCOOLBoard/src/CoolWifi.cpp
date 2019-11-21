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
#include <ESP8266WiFiMulti.h>
#include "internals/WiFiManagerReadFileButton.h"
#include <CoolWifi.h>
#include "ArduinoJson.h"


#define DEBUG 1


/**
*	CoolWifi::begin():
*	This method is provided to set the
*	wifiMulti Access points and the 
*	wifiManager time out 
*/
void CoolWifi::begin()
{ 

#if DEBUG == 1 

	Serial.println( F("Entering CoolWifi.begin()") );
	Serial.println();

#endif
	for(int i =0;i<this->wifiCount;i++)
	{
		 this->wifiMulti.addAP(this->ssid[i].c_str() , this->pass[i].c_str() );	
	}
	
}

/**
*	CoolWifi::state():
*	This method is provided to return the 
*	Wifi client's state.
*	\return wifi client state:	
*		WL_NO_SHIELD        = 255, 
*    		WL_IDLE_STATUS      = 0,
*    		WL_NO_SSID_AVAIL    = 1,
*    		WL_SCAN_COMPLETED   = 2,
*    		WL_CONNECTED        = 3,
*    		WL_CONNECT_FAILED   = 4,
*    		WL_CONNECTION_LOST  = 5,
*		WL_DISCONNECTED = 6
*/
wl_status_t CoolWifi::state()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolWifi.state()") );
	Serial.println();	
	Serial.print( F("state : ") );
	Serial.println( WiFi.status() );

#endif
	
	return( WiFi.status() ) ;
}

/**
*	CoolWifi::disconnect():
*	This method is provided to disconnect 
*   from current WiFi network and returns
*	the Wifi client's state.
*	\return wifi client state:	
*		WL_NO_SHIELD        = 255, 
*    		WL_IDLE_STATUS      = 0,
*    		WL_NO_SSID_AVAIL    = 1,
*    		WL_SCAN_COMPLETED   = 2,
*    		WL_CONNECTED        = 3,
*    		WL_CONNECT_FAILED   = 4,
*    		WL_CONNECTION_LOST  = 5,
*		WL_DISCONNECTED = 6
*/
wl_status_t CoolWifi::disconnect()
{

	WiFi.disconnect();

#if DEBUG == 1 

	Serial.println( F("Entering CoolWifi.disconnect()") );
	Serial.println();	
	Serial.print( F("state : ") );
	Serial.println( WiFi.status() );

#endif
	
	return( WiFi.status() ) ;
}

/**
*	CoolWifi::connect( ):
*	This method is provided to connect to the strongest WiFi
*	in the provided list of wiFis.
*	If none are found , it starts the AP mode.
*	
*	\return wifi state
*/
wl_status_t CoolWifi::connect()
{       



#if DEBUG == 1 

	Serial.println( F("Entering CoolWifi.connect()") );

#endif

	Serial.println( F("Wifi connecting...") );


	//if WifiCount > 0 , lunch wifiMulti
	//else no need to , skip this part
	// to wifiManager  
	if(this->wifiCount !=0)
	{
	
		this->connectWifiMulti();
		
		//if nomad is true, only check wifi list
		if(this->nomad == true)
		{
	
		#if DEBUG == 1
	
			Serial.print(F("nomad mode :"));
			Serial.println(this->nomad);
		
			Serial.print(F("Wifi status: "));
			Serial.println(WiFi.status());
		
		#endif	
			return(WiFi.status());	
		}

	} 
	else {
		WiFiManager wifiManager;
		wifiManager.resetSettings();
	}


	//Wifi Manager
	if( WiFi.status() != WL_CONNECTED ) 
	{

	#if DEBUG == 1 
	
		Serial.println(F("No matching wifi Found ") );
		Serial.println( F("Starting Access Point ") );	
		Serial.println();

	#endif
		
		this->connectAP();		
		
	}
	else
	{

		Serial.println(F("connected to "));
		Serial.println( WiFi.SSID() );
		Serial.println();
	
	}
	
	return( WiFi.status() ) ;

}

/**
*	CoolWifi::connectWifiMulti()
*	This function is provided to
*	run the WifiMulti part of the
*	Wifi connection process
*
*	\return wifi state
*/
wl_status_t CoolWifi::connectWifiMulti()
{
	int i=0;

#if DEBUG == 1 

	Serial.println(F("Entering CoolWifi.connectWifiMulti()"));
	Serial.println();
	
	Serial.println( F("entry time to multi : ") );
	Serial.println(millis() ) ;

#endif
	
	//Wifi MULTI
	while( (this->wifiMulti.run() != WL_CONNECTED) && (i<500)  ) 
	{

	#if DEBUG == 1

		Serial.print(F("."));
		
	#endif
		i++;
		delay(5);
    }	

#if DEBUG == 1 

	Serial.println();	
	Serial.println(F("exit point from multi : "));
	Serial.println(millis() );
	
	Serial.print(F("Wifi Status :"));
	Serial.println(WiFi.status());
#endif

	return(WiFi.status());

}


/**
*	CoolWifi::connectAP()
*	This function is provided to
*	run the WifiManager part of the
*	Wifi connection process
*
*	\return wifi state
*/
wl_status_t CoolWifi::connectAP()
{

#if DEBUG == 1 
	
	Serial.println( F("Entering CoolWifi.connectAP()") );	
	Serial.println();

#endif
	WiFiManager wifiManager;
	
	wifiManager.setRemoveDuplicateAPs(true);

	wifiManager.setTimeout(this->timeOut);
	
	String tempMAC = WiFi.macAddress();
	tempMAC.replace(":","");

	String name="CoolBoard-"+tempMAC;	

	if(!wifiManager.autoConnect(name.c_str())) 
	{

		Serial.println( F("failed to connect and hit timeout") );

		delay(30);

	} 

	//if you get here you have connected to the WiFi

	if(WiFi.status()==WL_CONNECTED)
	{

	#if DEBUG == 1

		Serial.println( F("connected...yeey :)" ));
		Serial.println("connected to ");
		Serial.println( WiFi.SSID() );
		//Serial.println( WiFi.psk() ) ;
		
	#endif

		this->addWifi( WiFi.SSID() , WiFi.psk() );
		
	}
	else
	{
		Serial.println( F("Not connected...:(" ));
	}
	
	return(WiFi.status());

}


/**
*	CoolWifi::config():
*	This method is provided to set
*	the wifi parameters :	-ssid
*				-pass
*				-AP timeOut
*				-wifiCount	
*
*	\return true if successful,false otherwise
*/
bool CoolWifi::config()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolWifi.config()") );
	Serial.println();

#endif
#if DEBUG == 0

	Serial.println( "Reading Wifi Configuration..");
	delay(100);
#endif 

	//read config file
	//update data
	File configFile = SPIFFS.open("/wifiConfig.json", "r");

	if (!configFile) 
	{
	
		Serial.println( F("failed to read /wifiConfig.json") );
		Serial.println();

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

			Serial.println( F("failed to parse json ") );
			Serial.println();

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
			//wifiCount
			if(json["wifiCount"].success() )
			{			
				this->wifiCount=json["wifiCount"];
			}
			else
			{
				this->wifiCount=this->wifiCount;
			}
			json["wifiCount"]=this->wifiCount;

			
			//AP timeOut
			if(json["timeOut"].success() )
			{
				this->timeOut=json["timeOut"];
			}
			else
			{
				this->timeOut=this->timeOut;

			}
			json["timeOut"]=this->timeOut;

			//nomad
			if(json["nomad"].success() )
			{
				this->nomad=json["nomad"];
			}
			else
			{
				this->nomad=this->nomad;

			}
			json["nomad"]=this->nomad;

			
			
			//Wifis SSID and PASS
			for(int i =0; i<this->wifiCount ;i++)
			{
				if ( json["Wifi"+String(i)].success() )
				{
					
					if( json["Wifi"+String(i)]["ssid"].success() )
					{
						const char* tempSsid=json["Wifi"+String(i)]["ssid"]; 
						this->ssid[i]=tempSsid;					
					}
					else
					{
						this->ssid[i]=this->ssid[i];					
					}
					json["Wifi"+String(i)]["ssid"]=this->ssid[i].c_str();
					
					
					if( json["Wifi"+String(i)]["pass"].success() )
					{
						const char* tempPass =json["Wifi"+String(i)]["pass"];
						this->pass[i]=tempPass ;					
					}
					else
					{
						this->pass[i]=this->pass[i];					
					}
					json["Wifi"+String(i)]["pass"]=this->pass[i].c_str();			
				
				}
				else
				{
					
					this->ssid[i]=this->ssid[i];
					this->pass[i]=this->pass[i];					
					
				}
				json["Wifi"+String(i)]["ssid"]=this->ssid[i].c_str();
				json["Wifi"+String(i)]["pass"]=this->pass[i].c_str();			
						
			}

			configFile.close();
			configFile = SPIFFS.open("/wifiConfig.json", "w");
			if(!configFile)
			{
			
				Serial.println( F("failed to write to /wifiConfig.json") );
			
				return(false);				
			}
			
			json.printTo(configFile);
			configFile.close();

		#if DEBUG == 1 

			Serial.println( F("saved configuration is :") );
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
*	CoolWifi::config(ssid array, pass array, number of wifis, AP timeout,nomad flag );
*	This method is provided to configure the Wifi without SPIFFS
*	
*	\return true if successfull, false otherwise
*/
bool CoolWifi::config(String ssid[],String pass[],int wifiNumber, int APTimeOut,bool nomad)
{

#if DEBUG == 1 
	
	Serial.println("Entering CoolWifi.config(), no SPIFFS variant ") ;
	
#endif
	
	if(wifiNumber>50)
	{
	
	#if DEBUG == 1 
		
		Serial.println("the limit of WiFis is 50 " );
		
	#endif
		return(false);	
	}

	this->wifiCount=wifiNumber;

	this->timeOut=APTimeOut;

	this->nomad=nomad;
	
	for(int i=0;i<wifiNumber;i++)
	{
		this->ssid[i]=ssid[i];
		
		this->pass[i]=pass[i];
	}
		
	return(true);

}


/**
*	CoolWifi::printConf():
*	This method is provided to print the
*	configuration to the Serial Monitor
*/
void CoolWifi::printConf()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolWifi.printConf()") );
	Serial.println();	

#endif
	
	Serial.println(F("Wifi configuration "));

	
	Serial.println(F("wifiCount : "));
	Serial.println(this->wifiCount);
	
	for(int i=0;i<this->wifiCount;i++)
	{	
		Serial.print(F("SSID"));
		Serial.print(i);
		Serial.println(F(" : "));
		Serial.println(this->ssid[i]);
				

		//Serial.print("PASS");
		//Serial.print(i);
		//Serial.println(" : ");

		//Serial.print(F("PASS"));
		//Serial.print(i);
		//Serial.println(F(" : "));

		//Serial.println(this->pass[i]);
		
	}
	
	Serial.println(F("timeOut : "));
	Serial.println(this->timeOut);

	Serial.println(F("nomad : "));
	Serial.println(this->nomad);

	Serial.println();


}

/**
*	CoolWifi::addWifi(ssid,pass)	
*	This method is provided to add new WiFi
*	detected by the WiFiManager to 
*	the jsonConfig(if used ) 
*	
*	\return true if successfull , false otherwise
*/
bool CoolWifi::addWifi( String ssid , String pass )
{

#if DEBUG == 1
	
	Serial.println("Entering CoolWifi.addWifi() ") ;

#endif 	
	
	this->wifiCount++;
	if( this->wifiCount >=50)
	{
	
	#if DEBUG == 1

		Serial.println("You have reached the limit of 50");
		return(false);	
	
	#endif

	}

	this->ssid[this->wifiCount-1]=ssid;
	this->pass[this->wifiCount-1]=pass;
	
	//read config file
	//update data
	File configFile = SPIFFS.open("/wifiConfig.json", "r");

	if (!configFile) 
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to read /wifiConfig.json") );
		Serial.println();

	#endif
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
			//wifiCount
			if(json["wifiCount"].success() )
			{			
				json["wifiCount"]=this->wifiCount;
			}
			else
			{
				this->wifiCount=this->wifiCount;
			}
			json["wifiCount"]=this->wifiCount;

			
			//AP timeOut
			if(json["timeOut"].success() )
			{
				this->timeOut=json["timeOut"];
			}
			else
			{
				this->timeOut=this->timeOut;

			}
			json["timeOut"]=this->timeOut;
			
			
			//new Wifi SSID and PASS
			JsonObject& newWifi = json.createNestedObject( "Wifi"+String( this->wifiCount-1 ) );
			
			newWifi["ssid"] =this->ssid[this->wifiCount-1];
			newWifi["pass"] = this->pass[this->wifiCount-1];
			

			configFile.close();
			configFile = SPIFFS.open("/wifiConfig.json", "w");
			if(!configFile)
			{
			
			#if DEBUG == 1 

				Serial.println( F("failed to write to /wifiConfig.json") );
			
			#endif

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

	
	return(true);
	
}
