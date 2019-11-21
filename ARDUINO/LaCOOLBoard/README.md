# README #

### What is this repository for? ###

Quick summary : 

CoolBoard API is a set of libraries and wrappers to simplify the access and usage of all 
the capabilites of the CoolBoard.

Version : 1.0

### How do I get set up? ###

Configuration :
 
 1/Download and Install the Arduino IDE (https://www.arduino.cc/en/Main/Software )
 
 2/Download and Add the ESP8266 Hardware extension to Arduino (https://github.com/esp8266/Arduino )
 
 /!\ If you already have Arduino IDE and the ESP8266 Hardware extension , make sure you upgrade them to the latest version.
 
 3/Download and Add the CoolBoard Library set to the Arduino IDE :

   a)First Method :
   
   -Open the Arduino IDE
	
   -Go to the "Sketch" Menu 
  
   -Include Library > Manage Libraries. 
  
   -Search for CoolBoard
   
   -Install
   
   -Restart Arduino IDE
   
   
   b)Second Method (if you have a CoolBoard.zip file ):
   
   -Open the Arduino IDE
   
   -Go to the "Sketch" Menu 
    
   -Include Library > "Add .ZIP Library". 
   
   -Search for CoolBoard.zip 
   
   -Click Open
   
   -Restart Arduino IDE
   
   
   
   c)Third Method (if you have the bitbucket/github link):
   
   -Open Arduino IDE > File > Preferences
   
   -Check the "SketchBook Location" path
   
   -Go to the Arduino/libraries folder (if it doesn't exist, create one )
   
   -Clone the repo there ( git clone "bitbucket/github link" )
   
   -Restart Arduino IDE
   
   
 4/Download ESP8266FS Tool (https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#uploading-files-to-file-system)
 
 5/Optional but Heavily Recommended : Download the ESP8266 Exception Decoder ( download link :https://github.com/me-no-dev/EspExceptionDecoder/releases/tag/1.0.6)
   Install guide : https://github.com/me-no-dev/EspExceptionDecoder
   
   
   
Dependencies :

You need the following libraries to be able to use the CoolBoard API:

-ArduinoJson(https://github.com/bblanchon/ArduinoJson)

-NeoPixelBus(https://github.com/Makuna/NeoPixelBus)

-TimeLib(https://github.com/PaulStoffregen/Time)

-DS1337RTC(https://github.com/etrombly/DS1337RTC)

-DallasTemperature(https://github.com/milesburton/Arduino-Temperature-Control-Library)

-OneWire(https://github.com/PaulStoffregen/OneWire)


Configuration Files :

-The CoolBoard API heavily uses the SPIFFS for storing and retreiving configuration and data files

This is a description of the configuration files and what are they used for :
  
  1/coolBoardConfig.json :
  
    logInterval: The time Interval to wait,in seconds, between two logs
	
    ireneActive: Put this flag to 1(true) if you are using the Irene3000 module
	
    jetpackActive:Put this flag to 1(true) if you are using the Jetpack modue
	
    externalSensorsActive: Put this flag to 1(true) if you are using a supported external Sensor
    
	sleepActive: Put this flag to 1(true) if you want your CoolBoard to enable Sleep mode
                 In Sleep Mode : your CoolBoard will do the following : -readSensors
                                                        				-activate Actors(if any)
																		-log the data
																		-check for updates
																		-go to sleep for logInterval period of time
	             
    userActive:Put this flag to 1(true) if you want your CoolBoard to collect userData : userName
																						 CoolBoard MAC Address 
																						 TimeStamp
                                                                                         
    manual /!\ : Put this flag to 1(true) to control to turn on/off your actors and bypass your initial actors configuration
                 
                 Be Careful with this mode!!.
                 
                 When this mode is actif , the CoolBoard will not restart automatically to apply any new configuration sent via
                 the update method.
                 
                 Restarting the coolBoard when in manual mode will deactivate all the actors!
                 
                 Never forget to reset this to false to go back to normal mode!


    saveAsJSON : Put this flag to one if you want that the coolboard saves messages which can not transmitted now in the SPIFFS 
    			 and sends them as soon as we have a connection. this is the best solution if you have no permanent connection.
    			 set it to zero if you have no internet at all and use the saveAsCSV Flag (smaller files).


    saveAsCSV :  This Flag saves the actual data as .csv file which can be opend in any openOffice, Excel...
    			 use this flag if you want to log for a long time without infrastructure around
                 
	
  2/coolBoardLedConfig.json:
  
    ledActive: Put this flag to 1(true) if you want to have Light Effects from the on Board LED
  	
  
  3/coolBoardSensorsConfig.json:

	temperature : Put this flag to 1(true) if you want to collect the Temperature using the BME280 Sensor

	humidity : Put this flag to 1(true) if you want to collect the humidity using the BME280 Sensor
	
	pressure : Put this flag to 1(true) if you want to collect the pressure using the BME280 Sensor
  
	visible : Put this flag to 1(true) if you want to collect the visible light index using the SI114X Sensor

	ir : Put this flag to 1(true) if you want to collect the infrared light index using the SI114X Sensor

	uv : Put this flag to 1(true) if you want to collect the ultraviolet light index using the SI114X Sensor

	vbat : Put this flag to 1(true) if you want to collect the battery voltage 
  
	soilMoisture : Put this flag to 1(true) if you want to collect the soil Moisture
	
	
  4/externalSensorsConfig.json:
  
	sensorsNumber: the number of supported external sensors you connect to the coolBoard 
	
	reference: the reference of a supported external sensor(e.g NDIR_I2C , DallasTemperature )
	
	type: the type of the measurments you are making (e.g : co2, temperature,voltage ... )
    
    address : the sensor's address , if it has one (e.g : NDIR_I2C CO2 sensor's address is 77 )

    kind0-kind4 : this flag permits ADS1015 and ADS1115 to give a name to Analog inputs kind0 is A0, kind1 is A1 and so on.


  5/irene3000Config.json:
  
	waterTemp.active: Put this flag to 1(true) in order to use the temperature sensor connected to the Irene3000

	phProbe.active: Put this flag to 1(true) in order to use the ph sensor connected to the Irene3000

	adc2.active: Put this flag to 1(true) in order to use the extra ADC input of the Irene3000
	
	adc2.gain: this is the value of the gain applied to the extra ADC input of the Irene3000
	
	adc2.type: the type of the measurments you are making (e.g : co2, temperature,voltage ... )


  6/jetPackConfig.json and coolBoardActorConfig.json:
  
  	Act[i].actif: Put this flag to 1(true) in order to use the jetpack output N�i (0..7)
		
	Act[i].inverted:Put this flag to 1(true) if the actor is inverted (e.g : a cooler is activated when Temp>TempMax)
					Put this flag to 0(false) if the actor is notInverted(e.g : a heater is activated when Temp<TempMin)
						
	Act[i].temporal:Put this flag to 1(true) if you want the actor to be actif of a period of time  ,
					then inactif for another period of time.
						
	Act[i].type:["primaryType","secondaryType"] : this array contains the priamryType and the secondaryType of the actor
												 -The primaryType is the type associated to the sensors.
												 (e.g : primaryType : "Temperature" is associated to the sensor of type "Temperature").
												 -It can also be empty , in case of PURE temporal actors
                                                 
                                                 
												 -The secondaryType is only used in temporal mode.
												 it can be : -"" (empty):the actor will be on for a period of timeHigh ms
												 			 			 the actor will be off for a period of timeLow ms
															 
															 -"hour" :the actor will be on when the Hour is equal or greater then hourHigh
															 		  the actor will be off when the Hour is equal or greater then hourLow
															 
															 -"minute":the actor will be on when the Minute is equal or greater then minuteHigh
															 		  :the actor will be off when the Minute is equal or greater then minuteLow
															 
															 -"hourMinute":the actor will be on when : Hour == hourHigh AND Minute >= minuteHigh
															 										   Hour > hourHigh
															 			   the actor will be off when : Hour == hourLow AND Minute >= minuteLow
															 			   								Hour>hourLow
                                                                                                        
                                                  /!\ NOTE  that if both primaryType and secondaryType are valid : the actor will be in
                                                     mixed mode : it will need to valid both Time and measurment conditions to go on or off
	
	Act[i].low:[rangeLow,timeLow,hourLow,minuteLow] : this array contains the values previously described:
													  -rangeLow is the minimum of the range at which 
													  to activate(deactivate in inverted mode) the actor
													  
													  -timeLow is the time spent off in temporal mode
													  
													  -hourLow is the hour to turn off the actor when secondaryType is hour or hourMinute
													  
													  -minuteLow is the minute to turn off the actor when secondaryType is minute or hourMinute
													  
													  
	Act[i].high:[rangeHigh,timeHigh,hourHigh,minuteHigh]: this array contains the values previously described:
													  -rangeHigh is the maximum of the range at which 
													  to deactivate(activate in inverted mode) the actor
													  
													  -timeHigh is the time spent on in temporal mode
													  
													  -hourHigh is the hour to turn on the actor when secondaryType is hour or hourMinute
													  
													  -minuteHigh is the minute to turn on the actor when secondaryType is minute or hourMinute
		
       
       Note that the coolBoardActorConfig.json contains only one Actor.

  7/mqttConfig.json:
  
  	mqttServer: This is the mqttServer (ip/url) address
  	
  	user: This is the userId
  	
  	bufferSize: This is the memory allocated to the mqtt buffer in bytes
	
	inTopic : this is the topic that the coolBoard subscribes to (receives updates from ). You don't need to configure this since it is autogenerated by the API.
	
	outTopic : this is the topic that the coolBoard will publish to. You don't need to configure this since it is autogenerated by the API.
	


  8/rtcConfig.json:

  	NTP : syncronizes the RTC over a NTP Pool. !! Disable this if you don't have internet !!
   
  	timeServer : NTP server DNS address. Find more information and the best server for your area here : http://www.pool.ntp.org
	
	localPort : port used to make the NTP request to update the time

	timeSync : the last time the Board updated the RTC (UNIX Time). The Coolboard trys to update unce per week, to adjust please look in CoolTime.h
	
	
  9/wifiConfig.json:
    
	wifiCount: the number of wifis saved in this configuration file

	timeOut:access point timeout in seconds.		
	
	nomad: put this flag to 1(true) to activate nomad mode.
		   in nomad mode : the coolBoard will only try to connect to known WiFis.
		   				   if it fails it will NOT lunch the access point.

 

  
How to run the exemples :
	
	-Open Arduino IDE 
	
	-File > Exemples > CoolBoard
	
	-Select the Exemple you want
	
	-Flash it
	
	-Flash the SPIFFS ( this is only required for the CoolBoardExemple, CoolBoardFarmExemple and CoolBoardStationExemple)
	
	-Open The Serial Monitor
	
	-Sit back and Enjoy!
	

### Contribution guidelines ###

For minor fixes of code and documentation, please go ahead and submit a pull request.

Larger changes (rewriting parts of existing code from scratch, adding new functions to the core, adding new libraries) 
should generally be discussed by opening an issue first.

Feature branches with lots of small commits (especially titled "oops", "fix typo", "forgot to add file", etc.) should be squashed before opening a pull request. 
At the same time, please refrain from putting multiple unrelated changes into a single pull request.


### License and credits ###

All files under src/internal are modified versions of existing libraries.
All Credit of the original work goes to their respective authors.

All Other files are provided as is under the MIT license :

Copyright (c) 2017 La Cool Co SAS

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


We can only gurantee that we did our best to have everything working on our side.


### Who do I talk to? ###

If you encounter a problem , have a good idea or just want to talk

Please open an issue, a pull request or send us an email :

La Cool Co <team@lacool.co>


