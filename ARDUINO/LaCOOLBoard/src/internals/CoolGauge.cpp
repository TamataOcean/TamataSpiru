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

#include <Arduino.h>
#include "ArduinoJson.h"
#include <math.h>    
#include <Wire.h>
#include "CoolGauge.h"

#define WIRE Wire

Gauges::Gauges(){}

void Gauges::getAllValues(){
	Wire.requestFrom(ADDR, 12);
	while(Wire.available()){
		for (int i = 0; i<=11; i++){
		this->rawData[i] = Wire.read();
		}
		for (int j = 0; j<=11; j++){
		}	}
	gauge1 = ( ((uint32_t)this->rawData[3] << 24)
     	+ ((uint32_t)rawData[2] << 16)
        + ((uint32_t)rawData[1] <<  8)
        + ((uint32_t)rawData[0] ) );  

	gauge2 = ( ((uint32_t)this->rawData[7] << 24)
     	+ ((uint32_t)rawData[6] << 16)
        + ((uint32_t)rawData[5] <<  8)
        + ((uint32_t)rawData[4] ) );  
	gauge3 = ( ((uint32_t)this->rawData[11] << 24)
     	+ ((uint32_t)rawData[10] << 16)
        + ((uint32_t)rawData[9] <<  8)
        + ((uint32_t)rawData[8] ) );  
		Wire.endTransmission();
}

uint32_t Gauges::readGauge1(){
     	Wire.requestFrom(ADDR, 12);
	while(Wire.available()){
		for (int i = 0; i<=11; i++){
		this->rawData[i] = Wire.read();
		}
	}
	gauge1 = (  ((rawData[3] & 0xff) << 24)  |
          ((rawData[2] & 0xff) << 16) 		 |
          ((rawData[1] & 0xff) << 8) 		 |
          (rawData[0] & 0xff) );  
	  return (gauge1);
}


uint32_t Gauges::readGauge2(){
     	 	Wire.requestFrom(ADDR, 12);
	while(Wire.available()){
		for (int i = 0; i<=11; i++){
		this->rawData[i] = Wire.read();
		}
	}
	gauge2 = (((rawData[7] & 0xff) << 24)  |
          ((rawData[6] & 0xff) << 16) 		 |
          ((rawData[5] & 0xff) << 8) 		 |
          (rawData[4] & 0xff) );  
	return (gauge2);
} 


uint32_t Gauges::readGauge3(){
     	 	Wire.requestFrom(ADDR, 12);
	while(Wire.available()){
		for (int i = 0; i<=11; i++){
		this->rawData[i] = Wire.read();
		}
	}
	gauge3 = ( ((uint32_t)this->rawData[11] << 24)
     	  + ((uint32_t)rawData[10] << 16)
        + ((uint32_t)rawData[9] <<  8)
        + ((uint32_t)rawData[8] ) );  
	  return (gauge3);
} 


void Gauges::resetGauge3(){
  Wire.beginTransmission(ADDR);  
  Wire.write(0x03);            
  Wire.endTransmission();      
}

void Gauges::resetGauge2(){
  Wire.beginTransmission(ADDR);           
  Wire.write(0x02);             
  Wire.endTransmission();     
}

void Gauges::resetGauge1(){
  Wire.beginTransmission(ADDR);                
  Wire.write(0x01);              
  Wire.endTransmission();    
}

void Gauges::resetAllValues(){
  Wire.beginTransmission(ADDR);                    
  Wire.write(0x00);              
  Wire.endTransmission();     
}
