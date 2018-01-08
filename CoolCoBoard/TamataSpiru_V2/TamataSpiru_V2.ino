/**
*	CoolBoard TamataSpiru
*
*	This example shows how typical use
*	of the CoolBoard.
*		
*	Save this example in another location
*	in order to safely modify the configuration files
*	in the data folder.
*
*/

#include <CoolBoard.h>


CoolBoard coolBoard;


void setup()
{
	Serial.begin(115200);
  Serial.println("-------- Tamata Spiruline Kit --------" );
  Serial.println("---- Managing Spirulina Growing  -----" );
  Serial.println("--------------------------------------" );
  
	coolBoard.config();
	coolBoard.begin();
  coolBoard.printConf();	
  
  /* Keeping Data */

  
	Serial.print("one log every ");
	Serial.print(coolBoard.getLogInterval());
	Serial.println(" s " );
}

void loop()
{
	if(coolBoard.isConnected()==0 )
	{
		coolBoard.onLineMode();
	}
	else
	{
		coolBoard.offLineMode();
	}
}
