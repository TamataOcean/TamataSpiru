/* Tamata Ocean - Open Source code for discovering */
/* Controlling Thermostat from Hard Code */
/* Necessary :
 *  Arduino UNO
 *  DS18B20 Sensor ( for water )
 *  4.7Kohm resistance
 *  Relay to control Heat
 *  Heater ( Resisitive Wire(2m/0.5Ohm) under the recipient )
 *  Electric Programmer to manager On / Off Time
 *  Alim 12V 2A 
 *  Breadboard & Wires
 */

#include <OneWire.h>
#include <DallasTemperature.h>

#define PIN_SENSOR_TEMP 5     
#define PIN_HEAT 6
#define TARGET_TEMP 30        // Temperature wanted during "On heat" period

OneWire oneWire_in(PIN_SENSOR_TEMP);
DallasTemperature sensor_temp(&oneWire_in);
float temperatureMC = 0;

void setup(void)
{
    pinMode(PIN_HEAT, OUTPUT);
    Serial.begin(9600);
    Serial.println("TamataSpiru Kit - V1.0");
    sensor_temp.begin();
}

void loop(void)
{
    Serial.print("Requesting temperatures...");
    sensor_temp.requestTemperatures();
    temperatureMC = sensor_temp.getTempCByIndex(0);

    Serial.println("Temperature : "+ String(temperatureMC ));

    /* Thermostat */
    if ( temperatureMC >= TARGET_TEMP ) {
      Serial.println("Heat Off");
      digitalWrite(PIN_HEAT, HIGH);
    }
    else{
      Serial.println("Heat On");
      digitalWrite(PIN_HEAT, LOW);
    }
    delay(5000);
}
