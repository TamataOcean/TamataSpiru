/* Modified by Mehdi Zemzem 
 *
 * CoolSI114X.cpp
 * Originally SI114X.cpp
 *
 *
 * A library for Grove - Sunlight Sensor v1.0
 *
 * Copyright (c) 2015 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : Fuhua.Chen
 * Modified Time: June 2015
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "CoolSI114X.h"
#include "Wire.h"
/*--------------------------------------------------------//
default init

 */
void CoolSI114X::DeInit(void)
{  
  //ENABLE UV reading  
  //these reg must be set to the fixed value
  WriteByte(CoolSI114X_UCOEFF0, 0x29);
  WriteByte(CoolSI114X_UCOEFF1, 0x89);
  WriteByte(CoolSI114X_UCOEFF2, 0x02);
  WriteByte(CoolSI114X_UCOEFF3, 0x00);
  WriteParamData(CoolSI114X_CHLIST, CoolSI114X_CHLIST_ENUV |CoolSI114X_CHLIST_ENALSIR | CoolSI114X_CHLIST_ENALSVIS |CoolSI114X_CHLIST_ENPS1);
  //
  //set LED1 CURRENT(22.4mA)(It is a normal value for many LED)
  //
  WriteParamData(CoolSI114X_PS1_ADCMUX, CoolSI114X_ADCMUX_LARGE_IR);
  WriteByte(CoolSI114X_PS_LED21, CoolSI114X_LED_CURRENT_22MA);
  WriteParamData(CoolSI114X_PSLED12_SELECT, CoolSI114X_PSLED12_SELECT_PS1_LED1); //
  //
  //PS ADC SETTING
  //
  WriteParamData(CoolSI114X_PS_ADC_GAIN, CoolSI114X_ADC_GAIN_DIV1);
  WriteParamData(CoolSI114X_PS_ADC_COUNTER, CoolSI114X_ADC_COUNTER_511ADCCLK);
  WriteParamData(CoolSI114X_PS_ADC_MISC, CoolSI114X_ADC_MISC_HIGHRANGE|CoolSI114X_ADC_MISC_ADC_RAWADC); 
  //
  //VIS ADC SETTING
  //
  WriteParamData(CoolSI114X_ALS_VIS_ADC_GAIN, CoolSI114X_ADC_GAIN_DIV1);
  WriteParamData(CoolSI114X_ALS_VIS_ADC_COUNTER, CoolSI114X_ADC_COUNTER_511ADCCLK);
  WriteParamData(CoolSI114X_ALS_VIS_ADC_MISC, CoolSI114X_ADC_MISC_HIGHRANGE);
  //
  //IR ADC SETTING
  //
  WriteParamData(CoolSI114X_ALS_IR_ADC_GAIN, CoolSI114X_ADC_GAIN_DIV1);
  WriteParamData(CoolSI114X_ALS_IR_ADC_COUNTER, CoolSI114X_ADC_COUNTER_511ADCCLK);
  WriteParamData(CoolSI114X_ALS_IR_ADC_MISC, CoolSI114X_ADC_MISC_HIGHRANGE);
  //
  //interrupt enable
  //
  WriteByte(CoolSI114X_INT_CFG, CoolSI114X_INT_CFG_INTOE);  
  WriteByte(CoolSI114X_IRQ_ENABLE, CoolSI114X_IRQEN_ALS);  
  //
  //AUTO RUN
  //
  WriteByte(CoolSI114X_MEAS_RATE0, 0xFF);
  WriteByte(CoolSI114X_COMMAND, CoolSI114X_PSALS_AUTO);
}

/*--------------------------------------------------------//
 Init the CoolSI114X and begin to collect data  


 */
bool CoolSI114X::Begin(void)
{
  Wire.begin(2,14);
  //
  //Init IIC  and reset si1145
  //
  if(ReadByte(CoolSI114X_PART_ID)!=0X45)
  {
    return false;
  }
  Reset();
  //
  //INIT 
  //
  DeInit();
  return true;
}
/*--------------------------------------------------------//
reset the CoolSI114X 
inclue IRQ reg, command regs...

 */
void CoolSI114X::Reset(void)
{
  WriteByte(CoolSI114X_MEAS_RATE0, 0);
  WriteByte(CoolSI114X_MEAS_RATE1, 0);
  WriteByte(CoolSI114X_IRQ_ENABLE, 0);
  WriteByte(CoolSI114X_IRQ_MODE1, 0);
  WriteByte(CoolSI114X_IRQ_MODE2, 0);
  WriteByte(CoolSI114X_INT_CFG, 0);
  WriteByte(CoolSI114X_IRQ_STATUS, 0xFF);

  WriteByte(CoolSI114X_COMMAND, CoolSI114X_RESET);
  delay(10);
  WriteByte(CoolSI114X_HW_KEY, 0x17);
  delay(10);
}
/*--------------------------------------------------------//
write one byte into CoolSI114X's reg

 */
void CoolSI114X::WriteByte(uint8_t Reg, uint8_t Value)
{
  Wire.beginTransmission(CoolSI114X_ADDR); 
  Wire.write(Reg); 
  Wire.write(Value); 
  Wire.endTransmission(); 
}
/*--------------------------------------------------------//
read one byte data from CoolSI114X

 */
uint8_t CoolSI114X::ReadByte(uint8_t Reg)
{
    Wire.beginTransmission(CoolSI114X_ADDR);
    Wire.write(Reg);
    Wire.endTransmission();
    Wire.requestFrom(CoolSI114X_ADDR, 1);  
    return Wire.read();
}
/*--------------------------------------------------------//
read half word(2 bytes) data from CoolSI114X

 */
uint16_t CoolSI114X::ReadHalfWord(uint8_t Reg)
{
  uint16_t Value;
  Wire.beginTransmission(CoolSI114X_ADDR);
  Wire.write(Reg); 
  Wire.endTransmission(); 
  Wire.requestFrom(CoolSI114X_ADDR, 2);
  Value = Wire.read();
  Value |= (uint16_t)Wire.read() << 8; 
  return Value;
}
/*--------------------------------------------------------//
read param data

 */
uint8_t CoolSI114X::ReadParamData(uint8_t Reg)
{
	WriteByte(CoolSI114X_COMMAND, Reg | CoolSI114X_QUERY);
	return ReadByte(CoolSI114X_RD);
}
/*--------------------------------------------------------//
writ param data

 */
uint8_t CoolSI114X::WriteParamData(uint8_t Reg,uint8_t Value)
{
	//write Value into PARAMWR reg first
   WriteByte(CoolSI114X_WR, Value);
   WriteByte(CoolSI114X_COMMAND, Reg | CoolSI114X_SET);
   //CoolSI114X writes value out to PARAM_RD,read and confirm its right
   return ReadByte(CoolSI114X_RD);
}

/*--------------------------------------------------------//
Read Visible Value

 */
 uint16_t CoolSI114X::ReadVisible(void)
{
  return ReadHalfWord(CoolSI114X_ALS_VIS_DATA0); 	
}
 /*--------------------------------------------------------//
Read IR Value

 */
 uint16_t CoolSI114X::ReadIR(void)
{
  return ReadHalfWord(CoolSI114X_ALS_IR_DATA0); 	
} 
/*--------------------------------------------------------//
Read Proximity Value

 */
 uint16_t CoolSI114X::ReadProximity(uint8_t PSn)
{
  return ReadHalfWord(PSn); 	
}
 /*--------------------------------------------------------//
Read UV Value
this function is a int value ,but the real value must be div 100

 */
uint16_t CoolSI114X::ReadUV(void)
{
  return (ReadHalfWord(CoolSI114X_AUX_DATA0_UVINDEX0)); 	
}
/*--------------------------------------------------------//
Read Response Register Value


 */
uint8_t  CoolSI114X::ReadResponseReg(void)
{
  return (ReadByte(CoolSI114X_RESPONSE)); 	
}
 
 
 
 
