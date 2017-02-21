/* Temperature Controller ( Control Temp with Sensor DS18B20 + Resistive wire to produce heat
 * Digital Print on LCD 16*2 caracters ( LCM1602C )
- Connect cable to Arduino Digital I/O Pin 2
- Connect Relay to Arduino
- Connect alternative powerplant
* V1.0 22/12/2016
contact@tamataocean.com */

/* -------------------- LIBRARIES ------------------------*/
#include <LiquidCrystal.h> // LCD 16*2 : LCM1602C )
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Timer.h>
#include <Wire.h>

/* ------------------------ I2C ---------------------------*/
#define SLAVE_ADDRESS 0x04
int resp = 0; //Answer on Serial Port to Raspberry
int command = 0; //command identifier (sent by Raspberry)
char str_temp[10];

/* -------------------- TEMPERATURE -----------------------*/
#define TEMP_CONTROL_WIRE_BUS 17     /*-(Temp Controller Connect to Pin 2 )-*/
#define TARGET_TEMPERATURE 35       /* Temperature Parameter */
OneWire ourWire(TEMP_CONTROL_WIRE_BUS);
DallasTemperature sensorTemp(&ourWire);
Timer Temperature;

/* --------------------- HEATING --------------------------*/
#define PIN_HEAT 14

/* ----------------- LCD Declaration ----------------------*/
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(13, 12, 5, 4, 3, 2); //RS=8 ,EN,D4,D5,D6,D7
Timer Lcd; int i=0;

// Special Caracter - http://alx.github.io/deuligne-custom-char/
int degreC = 0;
byte carDegreCelsius[8] = {
  B00111,B00101,B00111,B00000,B00000,B00000,B00000,};

int spiru=1;
byte carSpiru[8]={
  B01000,B00110,B00001,B00110,B01000,B00110,B10001,B01110};

int ok=2;
byte carOk[8]={
  B00100,B01010,B01010,B00100,B00000,B01010,B01100,B01010};

int up=3;
byte carUp[8]={
  B00111,B00011,B00101,B01000,B00000,B10111,B00100,B00111};

int sign=0;

/* -------------- COLOR SENSOR ---------------------------*/
Timer Color;
const int s0 = 8;
const int s1 = 9;
const int s2 = 7;
const int s3 = 6;
const int out = 10;

// Variables
int red = 0;
int green = 0;
int blue = 0;
float temperature = 0;

/* -------------- SETUP -------------- */
void setup() {
  Serial.begin(9600);
  // initialize i2c as slave
  Wire.begin(SLAVE_ADDRESS);

   //Initialize Temperature Sensor
  sensorTemp.begin();

  //Initialize Heat
  pinMode(PIN_HEAT, OUTPUT);

  delay(1000);
  Temperature.every(4000, voidTemp);
  Color.every(5000, voidColor);
  //Lcd.every(5000, voidLcd);

  //Special Caractere
  lcd.createChar(degreC, carDegreCelsius);
  lcd.createChar(spiru, carSpiru);
  lcd.createChar(ok, carOk);
  lcd.createChar(up, carUp);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Print Default message to the LCD.
  lcd.print("Spiru ");
  lcd.write(spiru);
  lcd.setCursor(12,0); //Jumping On Line
  lcd.write(degreC);
  lcd.print("C");

  //SetUp Color Sensor
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);
  digitalWrite(s0, HIGH);
  digitalWrite(s1, HIGH);

  // define callbacks for i2c
   Wire.onReceive(receiveData);
   Wire.onRequest(sendData);
}

void loop() {
  Temperature.update();
  Color.update();
  Lcd.update();
}

//Manage Temperature Checking,(under target -> heating on, else -> heating off)
void voidTemp(){
  sensorTemp.requestTemperatures(); // Send the command to get temperatures
  temperature = sensorTemp.getTempCByIndex(0);
  //Writing on LCD Temperature
  lcd.setCursor(8,0);
  lcd.print(temperature);

  if ( temperature > TARGET_TEMPERATURE ){
    digitalWrite(PIN_HEAT, HIGH);
    sign = ok;
    Serial.println("sign="+sign);
  } else {
    sign = up;
    Serial.println("sign="+sign);
    //Heating !
    digitalWrite(PIN_HEAT, LOW);
  }

  lcd.setCursor(15,0);
  lcd.write(sign);

}

//Manage Color Sensor
void voidColor()
{
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  //count OUT, pRed, RED
  red = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s3, HIGH);
  //count OUT, pBLUE, BLUE
  blue = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s2, HIGH);
  //count OUT, pGreen, GREEN
  green = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  //Print on LCD-Screen

  lcd.clear();
  lcdDefault();
  lcd.setCursor(0,1);
  lcd.print("R=");lcd.print(red);
  lcd.print("G=");lcd.print(green);
  lcd.print("B=");lcd.print(blue);

}

void lcdDefault(){
  lcd.setCursor(0,0);
  lcd.print("Spiru ");
  lcd.write(spiru);
  lcd.setCursor(8,0);
  lcd.print(temperature);
  lcd.setCursor(13,0); //Jumping On Line
  lcd.write(degreC);
  lcd.print("C");
  lcd.setCursor(15,0);
  lcd.write(sign);
}

/* ----- I2C : callback for received data from I2C ------- */
void receiveData(int byteCount){
 Serial.println("function receiveData: ");
 while(Wire.available()) {

     command = Wire.read();
     Serial.println("Command = "+String(command));
     //"2" is the command sent by Raspberry Pi in order to have the
     // temperature as answer on I2C from Arduino
     // Temperature Requested
     if(command==2) {
         //Arduino in this case sends the integer value of temperature
         dtostrf(temperature, 6, 2, str_temp);
         Serial.println("Temperature : "+ String(resp) );
     }
     // Color Red requested
     if (command==3) {
        dtostrf(red, 6, 2, str_temp);

     }
     // Color Green requested
     if (command == 4 ){
        dtostrf(green, 6, 2, str_temp);

     }
     // Color Blue requested
     if (command==5) {
        dtostrf(blue, 6, 2, str_temp);
     }

     // Thermosta Change Value requested
     if (command==6) {
        Serial.println("Thermosta Change requested");
        Serial.println("New Temperature Value : "+ TARGET_TEMPERATURE);     
     }
  }
}

void changeParameter(int byteCount, long value){}

// callback for sending data on I2C
void sendData(){
 Serial.println(resp);
 Wire.write(str_temp);
 Serial.println("Temperature "+String((int)temperature));
}

int getColor(int i) {
  if (i==3) {
     return red;
  }
  if (i==4) {
    return green;
  }
  if (i==5) {
    return blue;
  }
}
/*void voidLcd(){
  lcd.setCursor(9,1);
  if (i==0){
    lcd.display();
    lcd.print("R=");
    lcd.print(red);
    i++;
  }else if(i==1){
    lcd.print("G=");
    lcd.print(green);
    i++;
  }else {
    lcd.print("B=");
    lcd.print(blue);
    i=0;
  }
}*/
