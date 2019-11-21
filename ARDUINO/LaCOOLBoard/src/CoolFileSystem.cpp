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

#include <FS.h>
#include "CoolFileSystem.h"
#include "ArduinoJson.h"      // Arduino JSON File controller  https://github.com/bblanchon/ArduinoJson
#include "Arduino.h"


#define DEBUG 1


/**
*	CoolFileSystem::begin():
*	This method is provided to start the
*	SPIFFS object.
*
*	\return true if SPIFFS was initialized correctly,
*	false otherwise
*/
bool CoolFileSystem::begin()
{
	bool sResult=SPIFFS.begin();
	

#if DEBUG == 1

	Serial.println( F("Entering CoolFileSystem.begin()") );
	Serial.println();	
	Serial.print( F("SPIFFS success ? ") );
	Serial.println(sResult);
	Serial.println(  );

#endif
	//get the data from the files
	this->getsavedData();

	return( sResult  );                                   //Initialize Filesystem

}

/**
*	CoolFileSystem::saveSensorData( data ):
*	This method is provided to save the data on the local
*	memory when there is no internet available
*
*	increments the saved data flag  when successful
*
*	\return true if the data was saved,
*	false otherwise
*/
bool CoolFileSystem::saveSensorData(const char* data )
{

#if DEBUG == 1

	Serial.println( F("Entering CoolFileSystem.saveSensorData()") );
	Serial.println();

#endif
	
	File sensorsData=SPIFFS.open("/sensorsData.json","a");

	if(!sensorsData)
	{
	
	#if DEBUG == 1
	
		Serial.println( F("failed to append to /sensorsData.json") );
		Serial.println();
	
	#endif
		//keep the old data
		this->savedData=savedData;

		return (false);	
	}	

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(data);

	if( root.success() )
	{
		root.printTo(sensorsData);
		sensorsData.println();
		sensorsData.close();


	#if DEBUG == 1
		
		Serial.print(F("jsonBuffer size: "));
		Serial.println(jsonBuffer.size());
		Serial.println();

		sensorsData=SPIFFS.open("/sensorsData.json","r");
		
		if(!sensorsData)
		{
			
			Serial.println(F("failed to reopen /sensorsData.json"));
			return(true);
						
		}
	
		Serial.println( F("saved data is : ") );
		root.printTo(Serial);
		Serial.println();

		Serial.println(F("/sensorsData.json") );
		while (sensorsData.available()) 
		{
			Serial.println(sensorsData.readString()) ;
		}
		
		Serial.println();
		
		sensorsData.close();
	
	#endif

		//this->saveSensorDataCSV(data);		

		//new data
		this->savedData++;

		this->incrementsavedData();
		
		return (true);		
	}
	else
	{
	
	#if DEBUG == 1

		Serial.println( F("failed to parse json") );
	
	#endif
		//old data
		this->savedData=savedData;

		return(false);
	}
	

}


/**
*	CoolFileSystem::saveSensorDataCSV( data ):
*	This method is provided to save the data on the local
*	memory in CSV format.
*
*	\return true if the data was saved,
*	false otherwise
*/
bool CoolFileSystem::saveSensorDataCSV(const char* data )
{
#if DEBUG == 1

	Serial.println( F("Entering CoolFileSystem.saveSensorDataCSV()") );
	Serial.println();

#endif
	//parsing json
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(data);
	String header="",values="";
	
	//if json parse success
	if( root.success() )
	{		
		for (auto kv : root) 
		{
			//print the header(json keys ) to header string
			#if DEBUG ==1

				Serial.println(kv.key);
				Serial.println(kv.value.as<char*>() );

			#endif
			
			header+=kv.key;
			header+=',';
			
			//print the values to header string
			values+=( kv.value.as<char*>() );
			values+=',';
		}

		header.remove(header.lastIndexOf(','), 1);
		values.remove(values.lastIndexOf(','), 1);		
	
	#if DEBUG == 1
	
		Serial.println( F(" data is : ") );
		root.printTo(Serial);
		Serial.println();
		
		Serial.println(F(" header is :" ) ) ;
		Serial.println(header);
		Serial.println(F(" values are : "));
		Serial.println(values);
		
		Serial.print(F("jsonBuffer size: "));
		Serial.println(jsonBuffer.size());
		Serial.println();

	
	#endif
	
	}
	//failed to parse json
	else
	{
	
	#if DEBUG == 1

		Serial.println( F("failed to parse json") );
	
	#endif

		return(false);
	}

	//check if file exists
	File sensorsData=SPIFFS.open("/sensorsData.csv","r");
	
	//file doesn't exist
	if( (!sensorsData) || (sensorsData.size()==0 ) )
	{
	
	#if DEBUG == 1
	
		Serial.println( F("/sensorsData.csv not found") );
		Serial.println( F("creating /sensorsData.csv") );
		Serial.println();
	
	#endif
		//create file
		sensorsData=SPIFFS.open("/sensorsData.csv","w");
		
		if(!sensorsData)
		{

		#if DEBUG == 1
		
			Serial.println( F("failed to create /sensorsData.csv") );
			Serial.println();
		
		#endif
		
			return(false);

		}
		
		//print the header(json keys ) to the CSV file
		sensorsData.println(header);

		//print the values to the CSV file
		sensorsData.println(values);
		
		sensorsData.close();
	
	#if DEBUG == 1

		sensorsData=SPIFFS.open("/sensorsData.csv","r");
		
		if(!sensorsData)
		{
			Serial.println(F("failed to reopen /sensorsData.csv "));
			return(false);		
		}

		Serial.println( F("/sensorsData.csv : ") );

		while (sensorsData.available()) 
		{
  			Serial.print(sensorsData.readString()) ;
		}
		Serial.println();

		//close the file
		sensorsData.close();

	#endif
		

		
		return(true);
		
	}

	//file exist
	else
	{

	#if DEBUG == 1
	
		Serial.println( F("/sensorsData.csv  found") );
		Serial.println( F("appending to /sensorsData.csv") );
		Serial.println();
	
	#endif

		//append to file
		sensorsData=SPIFFS.open("/sensorsData.csv","a");
		
		if(!sensorsData)
		{
		
		#if DEBUG == 1
			
			Serial.println( F("failed to open /sensorsData.csv") );
			Serial.println();

		#endif
			
			return(false);
		
		}

		//print the values to the CSV file
		sensorsData.println(values);
		
		sensorsData.close();

	#if DEBUG == 1

		sensorsData=SPIFFS.open("/sensorsData.csv","r");
		
		if(!sensorsData)
		{
			Serial.println(F("failed to reopen /sensorsData.csv "));
			return(false);		
		}

		
		Serial.println( F("/sensorsData.csv : ") );

		while (sensorsData.available()) 
		{
  			Serial.println(sensorsData.readString()) ;
		}
		
		Serial.println();
		
		sensorsData.close();
		
	#endif		
		
		return(true);
	
	}	



}


/**
*	CoolFileSystem::updateConfigFiles( mqtt answer ):
*	This method is provided to update the configuration files when
*	the appropriate mqtt answer is received
*
*	\return true if the files are updated correctly,
*	false otherwise
*/
bool CoolFileSystem::updateConfigFiles(String answer )
{

	#if DEBUG == 1
	Serial.println( F("Entering CoolFileSystem.updateConfigFiles") );
	Serial.println();
	Serial.println( F("input answer : ") );
	Serial.println(answer);
	#endif

	//total json object	
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject( answer.c_str() )["desired"];

	#if DEBUG == 1
	Serial.println( F("json object : ") );	
	root.printTo(Serial);
	Serial.println();
	Serial.print(F("jsonBuffer size: "));
	Serial.println(jsonBuffer.size());
	Serial.println();
	#endif

	if(! ( root.success() ))
	{
		#if DEBUG == 1
		Serial.println( F("failed to parse root ") );
		Serial.println();
		#endif
		return(false);
	}
	else
	{
		#if DEBUG == 1
		Serial.println( F("success to parse root ") );
		Serial.println();
		root.printTo(Serial);
		#endif	
	}
	
	#if DEBUG == 1
	Serial.println( F("input message is : ") );
	root.printTo(Serial);
	Serial.println();
	#endif
	
	//temp string
	String temp;

	//--------- CoolBoard Configuration File
    JsonObject& jsonCoolBoard=root["CoolBoard"];
	#if DEBUG == 1
	Serial.println( F("before config CoolBoard json") );
	jsonCoolBoard.printTo(Serial);
	Serial.println();
	#endif
	
	if(jsonCoolBoard.success())
	{
		String update;
		jsonCoolBoard.printTo(update);
		this->fileUpdate(update,"/coolBoardConfig.json");		
	}
	else
	{
		#if DEBUG == 1 
		Serial.println( F("failed to parse CoolBoard ") );
		#endif
	}		

	//---------- Cool Board Sensors Configuration File
    JsonObject& jsonSensorsBoard=root["CoolSensorsBoard"];
	#if DEBUG == 1 
	Serial.println( F("before config CoolSensorsBoard json") );
	jsonSensorsBoard.printTo(Serial);
	#endif 
	
	if(jsonSensorsBoard.success())
	{	
		String update;
		jsonSensorsBoard.printTo(update);
		this->fileUpdate(update,"/coolBoardSensorsConfig.json");		
	}
	else
	{
		#if DEBUG == 1
		Serial.println( F("failed to parse CoolSensorsBoard sensors ") );	
		#endif
	}

	//---------- Cool Board Actor Configuration File
    JsonObject& jsonCoolBoardActor=root["CoolBoardActor"];
	#if DEBUG == 1 
	Serial.println( F("before config CoolBoardActor json") );
	jsonCoolBoardActor.printTo(Serial);
	#endif 
	
	if(jsonCoolBoardActor.success())
	{	
		String update;
		jsonCoolBoardActor.printTo(update);
		this->fileUpdate(update,"/coolBoardActorConfig.json");		
	}
	else
	{
		#if DEBUG == 1
		Serial.println( F("failed to parse CoolBoardActor json ") );	
		#endif
	}
	
	//----------- rtc configuration file
    JsonObject& jsonRTC=root["rtc"];
	#if DEBUG == 1 
	Serial.println( F("before config rtc json") );
	jsonRTC.printTo(Serial);
	#endif
	
	if(jsonRTC.success() )
	{
		String update;
		jsonRTC.printTo(update);
		this->fileUpdate(update,"/rtcConfig.json");			
	}
	else
	{
		#if DEBUG == 1 
		Serial.println( F("failed to parse rtc ") );
		#endif
	}
	
	
    //--------cool board led configuration
    JsonObject& jsonLedBoard=root["led"];

	#if DEBUG == 1 
	Serial.println( F("before config Led json") );
	jsonLedBoard.printTo(Serial);
	#endif

	if(jsonLedBoard.success())
	{	
		String update;
		jsonLedBoard.printTo(update);
		this->fileUpdate(update,"/coolBoardLedConfig.json");		
	}
	else
	{
		#if DEBUG == 1 
		Serial.println( F("failed to parse led") );
		#endif 
	}
		
	//------- jetpack configuration
    JsonObject& jsonJetpack=root["jetPack"];
	
	#if DEBUG == 1 
	Serial.println( F("before config jetpack json") );
	jsonJetpack.printTo(Serial);
	#endif

	if(jsonJetpack.success())
	{
		String update;
		jsonJetpack.printTo(update);
		this->fileUpdate(update,"/jetPackConfig.json");		
	}

	else
	{
		#if DEBUG == 1
		Serial.println( F("failed to parse jetpack") );	
		#endif
	}
	
	//------- irene configuration	
    JsonObject& jsonIrene=root["irene3000"];
	
	#if DEBUG == 1 
	Serial.println( F("before config irene json") );	
	jsonIrene.printTo(Serial);
	#endif 

	if(jsonIrene.success())
	{
		String update;
		jsonIrene.printTo(update);
		this->fileUpdate(update,"/irene3000Config.json");		
	}
	else
	{
		#if DEBUG == 1 
		Serial.println( F("failed to parse irene") );	
		#endif 
	}
	
	//------- external sensors
   	JsonObject& jsonExternalSensors=root["externalSensors"];

	#if DEBUG == 1 
	Serial.println( F("before config external Sensors json") );
	jsonExternalSensors.printTo(Serial);
	#endif

	if(jsonExternalSensors.success())
	{
		String update;
		jsonExternalSensors.printTo(update);
		this->fileUpdate(update,"/externalSensorsConfig.json");		
	}

	else
	{	
		#if DEBUG == 1
		Serial.println( F("failed to parse external sensors") );
		#endif
	}

	
	//------- mqtt config ----------
    JsonObject& jsonMQTT=root["mqtt"];
	
	#if DEBUG == 1 
	Serial.println( F("before config mqtt json") );
	jsonMQTT.printTo(Serial);
	#endif

	if(jsonMQTT.success())
	{
		String update;
		jsonMQTT.printTo(update);
		this->fileUpdate(update,"/mqttConfig.json");		
	}
	else
	{
		#if DEBUG == 1 
		Serial.println( F("failed to parse mqtt") );
		#endif
	}	

	//------- wifi config -----------
	JsonObject& jsonWifi=root["wifi"];
	#if DEBUG == 1 
	Serial.println( F("before config wifi json") );
	jsonWifi.printTo(Serial);
	#endif
	if(jsonWifi.success())
	{
		String update;
		jsonWifi.printTo(update);
		this->fileUpdate(update,"/wifiConfig.json");		
	}
	else
	{
		#if DEBUG == 1 
		Serial.println( F("failed to parse wifi") );
		#endif
	}	
		
	return true;

}	

/**
*	CoolFileSystem::isDataSaved():
*	This method is provided to report
*	wether there is sensor data saved in the
*	File System.
*
*	\return true if there is data saved, false
*	otherwise
*/
int CoolFileSystem::isDataSaved()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolFileSystem.isDataSaved()") );
	Serial.println();

	FSInfo fs_info;

	if(SPIFFS.info(fs_info)==true)
	{
		Serial.print(F("used bytes/total bytes : "));	
		Serial.print(fs_info.usedBytes);
		Serial.print(F("/"));
		Serial.print(fs_info.totalBytes);
		Serial.println();

	}

	File sensorsData=SPIFFS.open("/sensorsData.json","r");
	File sensorsDataCSV=SPIFFS.open("/sensorsData.csv","r");
	
	if( (!sensorsData)||(!sensorsDataCSV) )	
	{
		Serial.println( F("failed to open files") );		
	}
	else
	{		
			Serial.print(F("sensors Data file size in bytes : "));
			Serial.println(sensorsData.size());
			Serial.println();
			
			Serial.print(F("sensors Data CSV file size in bytes : "));				
			Serial.println(sensorsDataCSV.size());
			Serial.println();
	}



	Serial.print( F("savedData : ") );
	Serial.println(this->savedData);

#endif

	return( this->savedData );
}


/**
*	CoolFileSystem::getSensorData(int &lines):
*	This method is provided to return the 
*	sensor data saved in the File System
*	10 lines at a time

*	\return String array containing 
*	50 first lines from the file
*	modifies tge lines argument to reflect the number of
*	lines left
*/
String* CoolFileSystem::getSensorSavedData(int& lines)
{
	int maxString=50;
	String *sensorsDataArray=new String[maxString];
	lines=0;

#if DEBUG == 1 

	Serial.println( F("Entering CoolFileSystem.getSensorSavedData()") );
	Serial.println();

#endif

	//open sensors data file
	File sensorsData=SPIFFS.open("/sensorsData.json","r");
	
	if (!sensorsData)
	{

	#if DEBUG == 1 

		Serial.println( F("Failed to read /sensorsData.json") );

	#endif
		 
		sensorsDataArray[0]="Failed to read /sensorsData.json";
 		lines++;

		//result=sensorsDataArray;
		return(sensorsDataArray);

	}

	else
	{
		//read the file line by line and put it in the String array(aka String*)		

		//while loop until EOF is reached
		String temp;
		while(sensorsData.available())
		{
			yield();
			
			temp=sensorsData.readStringUntil('\r');

			if(linesToSkip>0)
			{

				linesToSkip--;
		
			}
			else
			{

			#if DEBUG == 1

				Serial.println(F("temp String : "));
				Serial.println(temp);
				Serial.println();
			
			#endif
			
				sensorsDataArray[lines]=temp;
				sensorsData.read();
			
			
		
			#if DEBUG== 1
	 
				Serial.print(F("read String N°"));
				Serial.print(lines);
				Serial.println(F(" is : "));
				Serial.println( sensorsDataArray[lines] );
				Serial.println();
				Serial.println(F("next char is : "));
				Serial.println((char)sensorsData.peek());
				Serial.println();			
			
			#endif
				lines++;
			
				//maximum size is maxString(index : 0..maxString-1)
				if(lines>=maxString)
				{
					break;
				}
			
				yield();
			
			}

		}
		
					
		//close the file
		sensorsData.close();

		//position the saved data flag to the number of unread lines
		this->savedData=savedData-lines;
		
		//position the number of lines to skip to the number of read lines	
		this->linesToSkip=lines;

		//delete data in the file only if savedData<=0
		if(this->savedData<=0)
		{
			File sensorsData=SPIFFS.open("/sensorsData.json","w");
			File sensorsDataCSV=SPIFFS.open("/sensorsData.csv","w");
			if( (!sensorsData)||(!sensorsDataCSV) )	
			{
			#if DEBUG == 1
	
				Serial.println( F("failed to delete data in the file") );
	
			#endif
				lines++;
				sensorsDataArray[lines]="failed to delete data in the file";

				return(sensorsDataArray);

			}

			sensorsData.close();
			sensorsDataCSV.close();
			
			this->linesToSkip=0;
		
		}

		//save the changes to linesToSkip and savedData in the file system
		this->incrementsavedData();

		//return the string
		#if DEBUG == 1
		
			for(int i=0;i<lines;i++)
			{
				Serial.print(F("String N°"));
				Serial.println(i);
				Serial.println(sensorsDataArray[i]);
				Serial.println(); 			
			}
	
		#endif
		return(sensorsDataArray);
		
		
		
	}

}

/**
*	CoolFileSystem::fileUpdate( update msg, file path):
*	This method is provided to ensure the 
*	correct update for each configuration file in the
*	File system
*
*	\return true if successful , false otherwise
*
*/
bool CoolFileSystem::fileUpdate(String update,const char* path)
{

#if DEBUG == 1

	Serial.println( F("Entering CoolFileSystem.fileUpdate()") );
	Serial.println();
	
	Serial.println(F("update msg is :"));
	Serial.println(update);
	
	Serial.println(F("file path is : "));
	Serial.println(path);	

#endif
	//transfer update String to json
	DynamicJsonBuffer updateBuffer;
	JsonObject& updateJson = updateBuffer.parseObject(update.c_str() );
	
	if(updateJson.success())
	{
	
	#if DEBUG ==1
		
		Serial.println(F("root parsing success :"));
		updateJson.printTo(Serial);
	
	#endif

	}
	else
	{
	
	#if DEBUG == 1 
	
		Serial.println(F("root parsing failure "));
	
	#endif
		
		return(false);	

	}
	
	//open file in read mode
	File configFile = SPIFFS.open( path , "r");
	
	if(!configFile)
	{	
	#if DEBUG == 1
		
		Serial.print( F("failed to read ") );
		Serial.println(path);

	#endif
		return(false);
	}

	//copy file to a json
	size_t size = configFile.size();

	// Allocate a buffer to store contents of the file.
	std::unique_ptr < char[] > buf(new char[size]);

	configFile.readBytes(buf.get(), size);

	DynamicJsonBuffer fileBuffer;

	JsonObject & fileJson = fileBuffer.parseObject(buf.get());

	if (!fileJson.success())
	{

	#if DEBUG == 1

		Serial.println( F("failed to parse json") );

	#endif

		return(false);
	}
	
	//modify root to contain all the json keys: updated ones and non updated ones
	for (auto kv : fileJson) 
	{
		if( updateJson[kv.key].success() )
		{
			fileJson[kv.key]=updateJson[kv.key];			
		}
		else
		{
			fileJson[kv.key]=fileJson[kv.key];
		}

				
	}

#if DEBUG == 1

	Serial.println(F("fileJson is now : "));
	fileJson.printTo(Serial);

#endif

	//close the file
	configFile.close();

	//open file in w mode
	configFile = SPIFFS.open( path , "w");
	
	if(!configFile)
	{	
	#if DEBUG == 1
		
		Serial.print( F("failed to open ") );
		Serial.println(path);

	#endif
		return(false);
	}
	//print json to file	
	
	fileJson.printTo(configFile);
	
	//close file
	configFile.close();


#if DEBUG == 1

	Serial.println( F("config is") );
	fileJson.printTo(Serial);
	Serial.println();

#endif
	
	return(true);
	
}



/**
*	CoolFileSystem::incrementsavedData():
*	This method is provided to increment the
*	savedData flag
*
*	\return true if successful , false otherwise
*
*/
bool CoolFileSystem::incrementsavedData()
{


#if DEBUG == 1
	
	Serial.println(F("Entering CoolFileSystem.incrementsavedData()"));	
	
#endif
	//open file
	File file=SPIFFS.open("/savedDataFlag.txt","w");	
	//read file
	if( !file)	
	{
		Serial.println( F("failed to open savedDataFlag.txt") );
		
		return(false);		
	}
	else
	{		
		//write to file	
		file.print(this->savedData);
		
		file.print(" ");
		
		file.println(this->linesToSkip);
	
		file.close();

	#if DEBUG == 1

		Serial.print(F("number of lines to read :"));
		Serial.println(this->savedData);
	
		Serial.print(F("number of lines to skip :"));
		Serial.println(this->linesToSkip);

	#endif
		
		return(true);
	}
	
}
/**
*	CoolFileSystem::getsavedData():
*	This method is provided to get the
*	savedData flag from the file system
*
*
*/
void CoolFileSystem::getsavedData()
{


#if DEBUG == 1
	
	Serial.println(F("Entering CoolFileSystem.getsavedData()"));	
	
#endif
	//open file
	File file=SPIFFS.open("/savedDataFlag.txt","r");	
	//read file
	if( !file)	
	{
		Serial.println( F("failed to read savedDataFlag.txt") );
		
	}
	else
	{		
		//read from file	
		String temp=file.readStringUntil(' ');
		
		this->savedData=temp.toInt();

		
		temp=file.readStringUntil('\n');
		this-> linesToSkip=temp.toInt();;	
		
		file.close();	
	}
	
#if DEBUG == 1

	Serial.print(F("number of lines to read :"));
	Serial.println(this->savedData);
	
	Serial.print(F("number of lines to skip :"));
	Serial.println(this->linesToSkip);

#endif
	
}
