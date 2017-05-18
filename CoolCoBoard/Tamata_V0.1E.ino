//#include <CoolBoard.h>

#include "Arduino.h"
#include <DS1337.h>
#include <NeoPixelBus.h>
#include <Wire.h>               //I2C support

#include "Adafruit_TCS34725.h"  // TCS34725 RGB Sensor
#include "SI114X.h"             //3 band lightsensor support
#include "SparkFunBME280.h"     //BME280 Support
#include <stdint.h>             //needed for the BME280

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
//CONTENT
#include "FS.h"
#include "QuickStats.h"
#include <ArduinoJson.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>

WiFiUDP udp;
IPAddress timeServerIP;                                                     // time.nist.gov NTP server address
//====================================================================
//const char* ntpServerName = "time.nist.gov";
const char* ntpServerName = "10.10.0.1";

//====================================================================
const int NTP_PACKET_SIZE = 48;                                             // NTP time stamp is in the first 48 bytes of the message
byte NTPBuffer[ NTP_PACKET_SIZE];                                           //buffer to hold incoming and outgoing NTP packets


#define colorSaturation 128
const uint16_t PixelCount = 1;  // this example assumes 4 pixels, making it smaller will cause a failure
const uint8_t PixelPin = 2;     // make sure to set this to the correct pin, ignored for Esp8266
#define TEMP_CONTROL_WIRE_BUS 0

OneWire ourWire(TEMP_CONTROL_WIRE_BUS);
DallasTemperature sensorTemp(&ourWire);

SI114X SI1145 = SI114X();
BME280 myBME280;

/* COLOR SENSOR  TCS34725 */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);
String colorHex = "";

WiFiClient espClient;
PubSubClient client(espClient);
DS1337 rtc;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

//coolboard coolboard;
QuickStats stats;

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

/*================== VARIABLES ===================== */
/*================================================== */
const bool DEBUG_FLAG = 1;
/* PROD COOLCO
  const char* mqtt_server = "52.17.46.139";
  const char* topicUpdate = "$aws/things/A020A608D635/shadow/update";
  const char* topicDelta = "$aws/things/A020A608D635/shadow/update/delta";*/

/* TAMATA RASPI MQTT / Topic Configuration */
const char* mqtt_server = "10.10.0.1";
const char* topicUpdate = "tamataraspi/spiru/update";
const char* topicDelta = "tamataraspi/spiru/update/delta";


uint8_t MAC_array[6];
char MAC_char[16];
char str_TE[16];                                // Exterior Temperature
char str_TE1[16];                               // DS18B20 Temperature
char str_HU[16];                                // Humidity
char str_SIG[4];
char str_PR[16];
char str_BAT[16];                               //Battery
char str_UV1[16];
char str_TIME[16];
char str_DATE[16];
char str_wifi[16];
int MO1 = 0;                                    //int for analogread
int VI1 = 0;                                    //Visible Light
float UV1 = 0;                                  //UV Light
int IR1 = 0;                                    //IR Light
long lastMsg = 0;
char msg[300];
char delta[300];
int value = 0;                                  //BME280 Pressure
float Temperature = 0;
float Temperature1 = 0;
float Humidity = 0;
float Pressure = 0;
float BAT = 0;                                  //Baterie tension

bool talkback = false;                          //config was modified
bool action = false;                            //do a action
bool onHeat = false;                            //used to know if Thermostat have to be on or off
bool onBubler = false;                          //used to know if Bubler is on or off
bool onLight = false;                           //used to know if Light is on or off

int INTERVAL = 10;                             //Sets wakeup interval in SECONDS in equal 15minutes = 900seconds, 1 hour = 3600seconds
unsigned long timeout;
unsigned long timef[4] = {0, 0, 0};
unsigned long laps;
unsigned long TIMESYNC;
String webString = "";
int EnMoisture = 13;
int AnMplex = 12;
int EnI2C = 5;                                  //double usage for I2C and shift register latch
int Pump = 15;
int clockPin = 4;
int dataPin = 15;

int MM = 1;
int PT = 20;
int P = 0;
int pinHeat = 5;
int pinLight = 6;
int pinBubler = 4;

byte Action = B11111111;
byte MANUAL = 0;

bool a = false;
byte Light = 0;
byte Heat = 0;
byte Act[8] = {0, 0, 0, 0, 0, 0, 0, 0};

int HeatMIN = 23;
int HeatMAX = 37;
int HeatTARGET = 30;
int HeatTIME = 600;

int HeatONhour = 7;
int HeatONminute = 00;
int HeatOFFhour = 23;
int HeatOFFminute = 00;

int LightONhour = 7;
int LightONminute = 00;
int LightOFFhour = 23;
int LightOFFminute = 00;

int BublerONhour = 7;
int BublerONminute = 00;
int BublerOFFhour = 23;
int BublerOFFminute = 00;

// INITIALIZE RTC AND VERIFY THE CLOCK ===============================
void rtcINIT() {
  bool trust = true;
  rtc.init();                                                               //initialise DS1337
  if (!rtc.isRunning()) {                                                   //if ever the RTC is stopped
    trust = false;                                                          //clock is not ok
    rtc.start();                                                            //start RTC
    Serial.println("no trust : not running");
  }
  if (rtc.hasStopped()) {                                                   //if the clock has stoped one moment or another
    trust = false;
    Serial.println("no trust : has stopped");
  }
  if (TIMESYNC + 604800 < rtc.getTimestamp()) {                             //once per week we try to get a time update
    trust = false;
    Serial.println("no trust : stopped");
  }
  if (trust == false) {
    getTIME();                                                              //synchronize Time
  }
}
// GET THE TIME ======================================================
void getTIME() {
  int cb = 0;
  bool err = false;
  byte count = 0;
  udp.begin(2390);                                                          //some random port for UDP communication
  WiFi.hostByName(ntpServerName, timeServerIP);
  sendNTPpacket(timeServerIP);                                              // send an NTP packet to a time server
  timeout = millis();
  while (!cb) {                                                             //check sunlight sensor
    cb = udp.parsePacket();
    delay(1000);
    if (millis() >= timeout + 3000) {                                       //retry every 3 seconds till receiving a packet
      WiFi.hostByName(ntpServerName, timeServerIP);                         //build hostname
      sendNTPpacket(timeServerIP);                                          // send an NTP packet to a time server
      timeout = millis();
      count++;
      Serial.println(count);
    }
    if (count >= 10) {                                                      //if we didn't get a answer after 10 seconds we break
      err = true;
      break;
    }
    count ++;
  }
  if (err == false) {                                                       //if we got a answer
    udp.read(NTPBuffer, NTP_PACKET_SIZE);                                   // read the packet into the buffer
    unsigned long highWord = word(NTPBuffer[40], NTPBuffer[41]);            //the timestamp starts at byte 40 of the received packet and is four bytes,
    unsigned long lowWord = word(NTPBuffer[42], NTPBuffer[43]);             // or two words, long. First, esxtract the two words
    TIMESYNC = highWord << 16 | lowWord;                                    // combine the four bytes (two words) into a long integer
    TIMESYNC = TIMESYNC - 2208988800UL;                                     // this is NTP time (seconds since Jan 1 1900) and we substract 70 years to get standard UNIX time
    rtc.setDateTime(TIMESYNC);                                              //set RTC to new time
    rtc.clearOSF();                                                         //since the sync worked fine we reset eventual error flags in the RTC
    talkback = true;                                                        //don't forget to save the date of the update in config.json
  }
  else Serial.println("Failed to syncronize with NTP service");             //just to say, maybe put a light animation...?
}
// CONSTRUCTION OF A NTP QUERY PACKET ================================
unsigned long sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);                                    // set all bytes in the buffer to 0. Initialize values needed to form NTP request (see URL above for details on the packets)
  NTPBuffer[0] = 0b11100011;                                                // LI, Version, Mode
  NTPBuffer[1] = 0;                                                         // Stratum, or type of clock
  NTPBuffer[2] = 6;                                                         // Polling Interval
  NTPBuffer[3] = 0xEC;                                                      // Peer Clock Precision
  NTPBuffer[12]  = 49;                                                      // 8 bytes of zero for Root Delay & Root Dispersion
  NTPBuffer[13]  = 0x4E;
  NTPBuffer[14]  = 49;
  NTPBuffer[15]  = 52;
  // all NTP fields have been given values, now you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123);                                            //NTP requests are to port 123
  udp.write(NTPBuffer, NTP_PACKET_SIZE);                                    //write buffer to stream
  udp.endPacket();
}


/////////////////////////////////////////////
//        Read On board Functions          //
/////////////////////////////////////////////
bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }
  MANUAL = json["MANUAL"];
  INTERVAL = json["INTERVAL"];
  TIMESYNC = json["TIMESYNC"];

  HeatMIN = json["HeatMIN"];
  HeatMAX = json["HeatMAX"];
  HeatTARGET = json["HeatTARGET"];
  HeatONhour = json["HeatONhour"];
  HeatONminute = json["HeatONminute"];
  HeatOFFhour = json["HeatOFFhour"];
  HeatOFFminute = json["HeatOFFminute"];

  LightONhour = json["LightONhour"];
  LightONminute = json["LightONminute"];
  LightOFFhour = json["LightOFFhour"];
  LightOFFminute = json["LightOFFminute"];

  BublerONhour = json["BublerONhour"];
  BublerONminute = json["BublerONminute"];
  BublerOFFhour = json["BublerOFFhour"];
  BublerOFFminute = json["BublerOFFminute"];

  Serial.println("--------- CONFIG ----------");
  Serial.print("Interval :  ");
  Serial.println(INTERVAL);
  Serial.print("Manual :  ");
  Serial.println(MANUAL);
  Serial.print("Heat ON :  ");
  Serial.print(HeatONhour); Serial.print("h");
  Serial.print(HeatONminute); Serial.println("min");
  Serial.print("Heat OFF :  ");
  Serial.print(HeatOFFhour); Serial.print("h");
  Serial.print(HeatOFFminute); Serial.println("min");
  Serial.print("HeatMIN :  ");
  Serial.println(HeatMIN);
  Serial.print("HeatMAX :  ");
  Serial.println(HeatMAX);
  Serial.print("HeatTARGET :  ");
  Serial.println(HeatTARGET);
  Serial.print("Light ON :  ");
  Serial.print(LightONhour); Serial.print("h");
  Serial.print(LightONminute); Serial.println("min");
  Serial.print("Light OFF :  ");
  Serial.print(LightOFFhour); Serial.print("h");
  Serial.print(LightOFFminute); Serial.println("min");
  Serial.print("Bubler ON :  ");
  Serial.print(BublerONhour); Serial.print("h");
  Serial.print(BublerONminute); Serial.println("min");
  Serial.print("Bubler OFF :  ");
  Serial.print(BublerOFFhour); Serial.print("h");
  Serial.print(BublerOFFminute); Serial.println("min");

  //Msg to show Config on Broker Topic
  snprintf(msg, 300, "{\"config\":{\"reported\":{\"User\":\"TamataSpiru\",\"Id\":\"%ld\",\"timestamp\":\"20%sT%sZ\",\"MANUAL\":%ld,\"INTERVAL\":%ld,\"TIMESYNC\":%ld,\"HeatMIN\":%ld,\"HeatMAX\":%ld,\"HeatTARGET\":%ld,\"HeatOn\":\"%ldH%ld\",\"HeatOff\":\"%ldH%ld\",\"LightOn\":\"%ldH%ld\",\"LightOff\":\"%ldH%ld\",\"BublerOn\":\"%ldH%ld\",\"BublerOff\":\"%ldH%ld\"}}}",
          MAC_char,
          str_DATE,
          str_TIME,
          MANUAL,
          INTERVAL,
          TIMESYNC,
          HeatMIN,
          HeatMAX,
          HeatTARGET,
          HeatONhour,
          HeatONminute,
          HeatOFFhour,
          HeatOFFminute,
          LightONhour,
          LightONminute,
          LightOFFhour,
          LightOFFminute,
          BublerONhour,
          BublerONminute,
          BublerOFFhour,
          BublerOFFminute);
  
  configFile.close();
}

bool saveConfig() {
  SPIFFS.remove("/config.json");
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["MANUAL"] = MANUAL;
  json["INTERVAL"] = INTERVAL;
  json["TIMESYNC"] = TIMESYNC;

  json["HeatMIN"] = HeatMIN;
  json["HeatMAX"] = HeatMAX;
  json["HeatTARGET"] = HeatTARGET;
  json["HeatONhour"] = HeatONhour;
  json["HeatONminute"] = HeatONminute;
  json["HeatOFFhour"] = HeatOFFhour;
  json["HeatOFFminute"] = HeatOFFminute;

  json["LightONhour"] = LightONhour;
  json["LightONminute"] = LightONminute;
  json["LightOFFhour"] = LightOFFhour;
  json["LightOFFminute"] = LightOFFminute;

  json["BublerONhour"] = BublerONhour;
  json["BublerONminute"] = BublerONminute;
  json["BublerOFFhour"] = BublerOFFhour;
  json["BublerOFFminute"] = BublerOFFminute;

  File configFile = SPIFFS.open("/config.json", "a+");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
  Serial.write(configFile);
  configFile.close();
  return true;
}

bool saveLASTlog() {
  SPIFFS.remove("/lastlog.json");
  File logFile = SPIFFS.open("/lastlog.json", "a+");
  if (!logFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  logFile.print(msg);
  //Serial.print("Save log to file : ");
  //Serial.println(msg);
  logFile.close();
  return true;
}

float readVbat() {
  digitalWrite(AnMplex, LOW);                                  //Enable Analog Switch to get the batterie tension
  delay(200);
  int raw = analogRead(A0);                                    //read in batterie tension
  //Serial.println(raw);
  float val = 6.04 / 1024 * raw;                               //convert it apprimatly right tension in volts
  return val;
}

int readMoisture() {
  digitalWrite(EnMoisture, HIGH);                 //enable moisture sensor and waith a bit
  digitalWrite(AnMplex, HIGH);
  delay(2000);
  int val = analogRead(A0);                       //read the value form the moisture sensor
  digitalWrite(EnMoisture, LOW);                  //disable moisture sensor for minimum wear
  return val;
}

void readBME() {
  Serial.println("---ReadBME---");/*
  TE[0] = myBME280.readTempC();
  PR[0] = myBME280.readFloatPressure();
  HU[0] = myBME280.readFloatHumidity();
  TE[1] = mySensor2.readTempC();
  PR[1] = mySensor2.readFloatPressure();
  HU[1] = mySensor2.readFloatHumidity();
  Temperature = stats.average(TE, 2);
  Humidity = stats.average(HU, 2);
  Pressure = stats.average(PR, 2);*/
  
  Temperature = myBME280.readTempC();
  Pressure = myBME280.readFloatPressure();
  Humidity = myBME280.readFloatHumidity();
  Serial.println("Temperature Ext : " + String(Temperature));
  Serial.println("Pressure : " + String(Pressure));
  Serial.println("Humidity : " + String(Humidity));

  }

void readSensors() {
  uint16_t red, green, blue, clearColor, colorTemp, lux;
  int wRed, wGreen, wBlue, wClear;
  String hexColor = "ffffff";

  SI1145.Reset();
  delay(20);
  SI1145.Begin();
  
  Serial.println(" ------------------- ReadSenors --------------------- ");
  Led(0, 128, 0, 10, 4);
  dtostrf(WiFi.RSSI(), 3, 0, str_SIG);
  VI1 = SI1145.ReadVisible();
  Serial.println("SI1145 : ---------------- ");
  Serial.println("VI1 : " + String(VI1));
  IR1 = SI1145.ReadIR();
  Serial.println("IR1 : " + String(IR1));
  UV1 = (float)SI1145.ReadUV();
  UV1 = UV1 / 100;
  Serial.println("UV1 : " + String(UV1));


  tcs.getRawData(&red, &green, &blue, &clearColor);  
  colorTemp = tcs.calculateColorTemperature(red, green, blue);
  lux = tcs.calculateLux(red, green, blue);
  Serial.println("TCS : -------------------- ");
  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.println("K - ");
  Serial.print("Lux: "); Serial.println(lux, DEC);
  Serial.print("R: "); Serial.print(red, DEC); Serial.print(" / ");
  Serial.print("G: "); Serial.print(green, DEC); Serial.print(" / ");
  Serial.print("B: "); Serial.print(blue, DEC); Serial.print(" / ");
  Serial.print("C: "); Serial.print(clearColor, DEC); Serial.println(" ");

  uint32_t sum = clearColor;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  colorHex = "#" + String((int)r, HEX) + String((int)g, HEX) + String((int)b, HEX);
  Serial.println("ColorHex : "+colorHex);
  
  Serial.println("BME : -------------------- ");
  Temperature = myBME280.readTempC();
  Pressure = myBME280.readFloatPressure();
  Humidity = myBME280.readFloatHumidity();
  Serial.println("Temperature Ext : " + String(Temperature));
  Serial.println("Pressure : " + String(Pressure));
  Serial.println("Humidity : " + String(Humidity));

  Serial.println("DS18B20 : ---------------- ");
  // Get TEmperature MC ( inside water )
  sensorTemp.requestTemperatures(); // Send the command to get temperatures
  Temperature1 = sensorTemp.getTempCByIndex(0);
  Serial.println("Temperature MC : " + String(Temperature1));
  
  rtc.getDate().getTimeString().toCharArray(str_TIME, 16);
  rtc.getDate().getDateString().toCharArray(str_DATE, 16);
  dtostrf(UV1, 4, 2, str_UV1);
  dtostrf(Temperature1, 4, 2, str_TE1);
  dtostrf(Temperature, 4, 2, str_TE);
  dtostrf(Humidity, 4, 2, str_HU);
  dtostrf(Pressure, 6, 0, str_PR);
  //dtostrf(BAT, 4, 2, str_BAT);
  
  onHeat = bitRead(Action, pinHeat);
  onLight = bitRead(Action, pinLight);
  onBubler = bitRead(Action, pinBubler);

  snprintf(msg, 600, "{\"state\":{\"reported\":{\"User\":\"TamataSpiru\",\"Id\":\"%s\",\"timestamp\":\"20%sT%sZ\",\"Temp\":%s,\"Temp1\":%s,\"Humi\":%s,\"Pres\":%s,\"Vis\":%ld,\"IR\":%ld,\"UV\":%s,\"SIG\":%s,\"HeatTARGET\":%ld,\"r\":%ld,\"g\":%ld,\"b\":%ld,\"lux\":%ld,\"colorTemp\":%ld,\"colorHex\":\"%s\",\"onHeat\":%ld,\"onLight\":%ld,\"onBubler\":%ld}}}",
           MAC_char,
           str_DATE,
           str_TIME,
           str_TE,
           str_TE1,
           str_HU,
           str_PR,
           VI1,
           IR1,
           str_UV1,
           str_SIG,
           HeatTARGET,
           red,
           green,
           blue,
           lux,
           colorTemp,
           colorHex.c_str(),
           onHeat,
           onLight,
           onBubler);
}

void doAction(byte actuator, bool state) {
  bitWrite(Action, actuator, state);
  Serial.print("Act :"); Serial.print(actuator);
  Serial.print("state :"); Serial.println(state);
  digitalWrite(EnI2C, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, Action);
  Serial.println(Action, BIN);
  digitalWrite(EnI2C, HIGH);
}

/////////////////////////////////////////////
//         Wifi & MQTT Functions           //
/////////////////////////////////////////////

void autoWIFI() {
  File a = SPIFFS.open("/WIFIconfig.json", "r");
  if (!a) {
    //create file and start server for configuration
  }

  a.readStringUntil(':');
  int num = a.readStringUntil(',').toInt();                   //read number of WIFI networks saved in the memory
  //Serial.println(num);
  String ssid[num];                                           //placeholder for SSID's in the memory
  String password[num];                                       //placeholder for Passwords in the memory
  boolean match[num];                                         //is true when a nearby WIFI is in the memory
  int matchNum = 999;                                         //needed placeholder for the Wifi List
  float matchSig[num];                                        //for Wifi strenght analisis
  float bestSig = -300 ;                                      //value for the strongest Wifi signal

  for (int i = 0; i <= num - 1; i++) {                        //initialize values and read networks & password to the memory
    a.readStringUntil(':');                                   //parsing is done "by hand" since arduinoJSON creates impossible bugs..
    a.readStringUntil('"');
    match[i] = false;                                         //reset all match flags since we need to start from blank
    ssid[i] = a.readStringUntil('"');                         //read SSID in memory
    a.readStringUntil('"');
    password[i] = a.readStringUntil('"');                     //read password into memory
    matchSig[i] = -300;                                       //set a default value below WiFi capabilities of the ESP for signal strenght detection
    //Serial.println(ssid[i]);
    //Serial.println(password[i]);
  }
  int numWIFI = WiFi.scanNetworks();                          //scan number of networks
  if (numWIFI == 0) {                                         //no network found --> start webserver
    Led(256, 0, 0, 0, 5);
  }
  else {
    for (int k = 0; k <= numWIFI - 1 ; k++)                   //scan WIFI list for matches
    {
      for (int j = 0; j <= num ; j++) {
        //Serial.println(j);
        if (WiFi.SSID(k) == ssid[j])                          //Compare to List in memory
        {
          match[j] = true;                                    //set match flag
          matchSig[j] = (float)WiFi.RSSI(k);                  //note signal strenght
          matchNum = k;
        }
      }
    }
    if (matchNum == 999) {
      Led(256, 0, 0, 100, 5);
      //WIFIconfig();
      //Serial.print("matchNum = ");
      //Serial.println(matchNum);
    }
  }
  bestSig = stats.maximum(matchSig, num);
  for (int l = 0; l <= num - 1; l++) {
    if (matchSig[l] == bestSig) {
      const char *thisNET = ssid[l].c_str();
      const char *thisPASS = password[l].c_str();
      Serial.println(thisNET);
      WiFi.persistent(false);
      WiFi.mode(WIFI_OFF);   // this is a temporary line, to be removed after SDK update to 1.5.4
      WiFi.mode(WIFI_STA);
      WiFi.begin(thisNET, thisPASS);
      while (WiFi.status() != WL_CONNECTED)  {
        Led(70, 0, 255, 3, 1);
        Serial.print(".");
      }
      Led(0, 128, 0, 10, 3);
      return;
    }
  }
}

void sendMessage() {
  while (!client.connected()) {                        //connect to the broker
    reconnect();
  }
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish(topicUpdate, msg);   //publish message
  timeout = millis();
  while (talkback == false) {                                         //wait till you got the feedback
    client.loop();
    yield();
    if (millis() > timeout + 3000) {
      break;
    }
  }
  if (talkback == true) {
    if (!saveConfig()) {
      Serial.println("Failed to save config");
    } else {
      Serial.println("Config saved");
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String order = "";
  talkback = false;
  StaticJsonBuffer<300> jsonBuffer;
  Serial.print("Message arrived [");
  Serial.print(topic);
  //Serial.print(" payload : ");
  //Serial.println(payload);
  Serial.print("] ");
  snprintf(delta, 300, "%s", payload);
  Serial.println("Delta = " + String(delta));
  Serial.println();
  JsonObject& root = jsonBuffer.parseObject(delta);
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return;
  }
  if (root["state"]["INTERVAL"] != 0) {
    INTERVAL = root["state"]["INTERVAL"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"INTERVAL\":%ld},\"desired\":null}}}", INTERVAL);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["LightONhour"] != -1) {
    LightONhour = root["state"]["LightONhour"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"LightONhour\":%ld},\"desired\":null}}}", LightONhour);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["LightONminute"] != -1) {
    LightONminute = root["state"]["LightONminute"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"LightONminute\":%ld},\"desired\":null}}}", LightONminute);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["LightOFFhour"] != -1) {
    LightOFFhour = root["state"]["LightOFFhour"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"LightOFFhour\":%ld},\"desired\":null}}}", LightOFFhour);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["LightOFFminute"] != -1) {
    LightOFFminute = root["state"]["LightOFFminute"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"LightOFFminute\":%ld},\"desired\":null}}}", LightOFFminute); 
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["HeatMIN"] != 0) {
    HeatMIN = root["state"]["HeatMIN"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"HeatMIN\":%ld},\"desired\":null}}}", HeatMIN);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["HeatMAX"] != 0) {
    HeatMAX = root["state"]["HeatMAX"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"HeatMAX\":%ld},\"desired\":null}}}", HeatMAX); 
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["HeatTARGET"] != 0) {
    HeatTARGET = root["state"]["HeatTARGET"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"HeatTARGET\":%ld},\"desired\":null}}}", HeatTARGET);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["HeatONhour"] != -1) {
    HeatONhour = root["state"]["HeatONhour"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"HeatONhour\":%ld},\"desired\":null}}}", HeatONhour);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["HeatOFFhour"] != -1) {
    HeatOFFhour = root["state"]["HeatOFFhour"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"HeatOFFhour\":%ld},\"desired\":null}}}", HeatOFFhour);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["HeatONminute"] != -1) {
    HeatONminute = root["state"]["HeatONminute"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"HeatONminute\":%ld},\"desired\":null}}}", HeatONminute);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["HeatOFFminute"] != -1) {
    HeatOFFminute = root["state"]["HeatOFFminute"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"HeatOFFminute\":%ld},\"desired\":null}}}", HeatOFFminute);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["HeatONhour"] != -1) {
    HeatONminute = root["state"]["HeatONminute"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"HeatONhour\":%ld},\"desired\":null}}}", HeatONhour);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["HeatOFFhour"] != -1) {
    HeatOFFminute = root["state"]["HeatOFFhour"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"HeatOFFhour\":%ld},\"desired\":null}}}", HeatOFFhour);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["BublerONhour"] != -1) {
    BublerONhour = root["state"]["BublerONhour"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"BublerONhour\":%ld},\"desired\":null}}}", BublerONhour);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["BublerOFFhour"] != -1) {
    BublerOFFhour = root["state"]["BublerOFFhour"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"BublerOFFhour\":%ld},\"desired\":null}}}", BublerOFFhour);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["BublerONminute"] != -1) {
    BublerONminute = root["state"]["BublerONminute"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"BublerONminute\":%ld},\"desired\":null}}}", BublerONminute);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }
  if (root["state"]["BublerOFFminute"] != -1) {
    BublerOFFminute = root["state"]["BublerOFFminute"];
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"BublerOFFminute\":%ld},\"desired\":null}}}", BublerOFFminute);
    Serial.println(msg);
    client.publish(topicUpdate, msg);
    talkback = true;
  }


  for (int i = 1; i <= 8; i++) {
    snprintf(msg, 10, "Act%ld", i);
    //Serial.println(msg);

    if (root["state"][msg] != 0) {
      Act[i - 1] = root["state"][msg];
      if (Act[i - 1] == 4) {  //Act4 = Move / Act6 = Light / Act5 = Heat 
        doAction(i, 1);
        snprintf(msg, 300, "{\"state\":{\"reported\":{\"Act%ld\":1},\"desired\":null}}}", i);
        Serial.println(msg);
        client.publish(topicUpdate, msg);
      }
      if (Act[i - 1] == 3) {
        doAction(i, 0);
        snprintf(msg, 300, "{\"state\":{\"reported\":{\"Act%ld\":0},\"desired\":null}}}", i);
        Serial.println(msg);
        client.publish(topicUpdate, msg);
      }
      talkback = true;
    }
  }

  if (root["state"]["MANUAL"] != 0) {
    MANUAL = root["state"]["MANUAL"];
    Serial.println(MANUAL);
    if (MANUAL == 4) {
      MANUAL = 1;
      snprintf(msg, 300, "{\"state\":{\"reported\":{\"MANUAL\":1},\"desired\":null}}}");
      Serial.println(msg);
      client.publish(topicUpdate, msg);
    }
    if (MANUAL == 3) {
      MANUAL = 0;
      snprintf(msg, 300, "{\"state\":{\"reported\":{\"MANUAL\":0},\"desired\":null}}}");
      Serial.println(msg);
      client.publish(topicUpdate, msg);
    }
    talkback = true;
  }
  if (talkback == true) {
    if (!saveConfig()) {
      Serial.println("Failed to save config");
    } else {
      Serial.println("Config saved");
      talkback = false;
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MAC_char)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("topic/test", "hello world");
      // ... and resubscribe
      client.subscribe(topicDelta);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//////////////////////////////////////////////
// Miscalenius Functions & Light animations //
//////////////////////////////////////////////

void Led(int R, int G, int B, int T, int A) {
  if (DEBUG_FLAG == 0) {
    switch (A) {
      case 1:
        ColorFade(R, G, B, T);
        break;
      case 2:
        OnOff(R, G, B, T);
        break;
      case 3:
        FadeIn(R, G, B, T);
        break;
      case 4:
        FadeOut(R, G, B, T);
        break;
      case 5:
        Strobe(R, G, B, T);
        break;
    }
  }
  else delay(100 * T);
}

void ColorFade(int R, int G, int B, int T) {
  for (int k = 0; k < 100; k++) {
    strip.SetPixelColor(0, RgbColor(k * R / 100, k * G / 100, k * B / 100));
    strip.Show();
    delay(T);
  }
  // Fade OUT
  for (int k = 100; k >= 0; k--) {
    strip.SetPixelColor(0, RgbColor(k * R / 100, k * G / 100, k * B / 100));
    strip.Show();
    delay(T);
  }
}

void OnOff(int R, int G, int B, int T) {
  strip.SetPixelColor(0, RgbColor(R, G, B));
  strip.Show();
  delay(T);
  strip.SetPixelColor(0, RgbColor(0, 0, 0));
  strip.Show();
}

void FadeIn(int R, int G, int B, int T) {
  for (int k = 0; k < 100; k++) {
    strip.SetPixelColor(0, RgbColor(k * R / 100, k * G / 100, k * B / 100));
    strip.Show();
    delay(T);
  }
}
void FadeOut(int R, int G, int B, int T) {
  for (int k = 100; k >= 0; k--) {
    strip.SetPixelColor(0, RgbColor(k * R / 100, k * G / 100, k * B / 100));
    strip.Show();
    delay(T);
  }
}

void Strobe(int R, int G, int B, int T) {
  for (int k = 5; k >= 0; k--) {
    strip.SetPixelColor(0, RgbColor(R, G, B));
    strip.Show();
    delay(T);
    strip.SetPixelColor(0, RgbColor(0, 0, 0));
    strip.Show();
    delay(T);
  }
}

void gotosleep() {
  Serial.print("GOTO SLEEP for ");
  Serial.print(INTERVAL / 60);
  Serial.println(" minutes");
  Led(128, 128, 128, 3, 3);
  ESP.deepSleep((INTERVAL * 1000000), WAKE_RF_DEFAULT);               //set deepsleep periode in seconds
}


void showInfo() {
  Serial.println("INFO");
  Serial.println("----");

  // clock
  Serial.print("CLOCK: ");
  // time
  Serial.print(rtc.getDate().getTimeString());
  Serial.print(" - ");
  // date
  Serial.print(rtc.getDate().getDateString());
  // oscillator
  if (!rtc.isRunning())
    Serial.print(" STOPPED");
  else
    Serial.print(" RUNNING");
  if (rtc.hasStopped())
    Serial.print(" HAS STOPPED");
  Serial.println();

  // alarm
  Serial.print("ALARM: ");
  Serial.print(rtc.getAlarm().getTimeString());
  Serial.print("@");
  Serial.print(rtc.getAlarm().getDay());
  if (rtc.isAlarmEnabled())
    Serial.print(" ENABLED");
  else
    Serial.print(" DISABLED");
  if (rtc.isAlarmActive())
    Serial.print(" ACTIVE");
  switch (rtc.getAlarmMode()) {
    case DS1337_ALARM_EVERY_SECOND: Serial.print(" - ON EVERY SECOND"); break;
    case DS1337_ALARM_ON_SECOND: Serial.print(" - ON SECOND"); break;
    case DS1337_ALARM_ON_SECOND_MINUTE: Serial.print(" - ON SECOND/MINUTE"); break;
    case DS1337_ALARM_ON_SECOND_MINUTE_HOUR: Serial.print(" - ON SECOND/MINUTE/HOUR"); break;
    case DS1337_ALARM_ON_SECOND_MINUTE_HOUR_DATE: Serial.print(" - ON SECOND/MINUTE/HOUR/DATE"); break;
    case DS1337_ALARM_ON_SECOND_MINUTE_HOUR_DAY: Serial.print(" - ON SECOND/SECOND/MINUTE/HOUR/DAY"); break;
    default: Serial.print(" - UNKNOWN");
  }
  Serial.println();

  // tick mode
  int tickMode = rtc.getTickMode();
  Serial.print("TICKS: ");
  switch (tickMode) {
    case DS1337_NO_TICKS: Serial.println("OFF"); break;
    case DS1337_TICK_EVERY_SECOND: Serial.println("EVERY SECOND"); break;
    case DS1337_TICK_EVERY_MINUTE: Serial.println("EVERY MINUTE"); break;
    case DS1337_TICK_EVERY_HOUR: Serial.println("EVERY HOUR"); break;
    default: Serial.println("UNKNOWN");
  }
  Serial.println();
}
/*===================================================================================*/
/*==================================== SETUP ========================================*/
/*===================================================================================*/
//CoolBoard board;
void setup() {
  pinMode(AnMplex, OUTPUT);                         //Declare Analog Multiplexer OUTPUT
  pinMode(EnMoisture, OUTPUT);                      //Declare Moisture enable Pin
  pinMode(EnI2C, OUTPUT);
  pinMode(Pump, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  digitalWrite(AnMplex, LOW);                       //put the multiplexer low (Vbat)
  digitalWrite(EnMoisture, LOW);
  digitalWrite(EnI2C, HIGH);
  digitalWrite(Pump, LOW);
  for (int i = 0; i <= 7; i++) {
    doAction(i, 0);
  }
  if (DEBUG_FLAG == 0) {                            //see if the debug Flag was set
    strip.Begin();                                  //if not start WS2812 led
    strip.SetPixelColor(0, RgbColor(0, 0, 0));      //cut the light
    strip.Show();                                   //push pixel
  }
  else {
    Serial.begin(115200);                           //If the debug was set enable Serial
    delay(2000);
  }
  SPIFFS.begin();                                   //Initialize Filesystem
  loadConfig();                                     //load parameters from SPIFFS
  Wire.begin(2, 14);                                //I2C init
  WiFi.macAddress(MAC_array);                       //Get mac adresse and store it for later
  for (int i = 0; i < sizeof(MAC_array); ++i) {
    sprintf(MAC_char, "%s%02x", MAC_char, MAC_array[i]);
  }
  Serial.println(MAC_char);

  if (tcs.begin()) {
    Serial.println("Tcs Sensors Found");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
  }
  
  /* Get Object From COOL_Board.h -- Not Working at this moment...
  board.setDebugFlag (DEBUG_FLAG);
  board.registerSensor ("DS1337");                                          // RTC
  board.registerSensor ("battery");
  board.registerSensor ("SparkFunBME280", 0x76);                            // humity, temperature, pressure
  board.registerSensor ("Si1145");                                          // visible light, UV, IR
  board.registerSensor ("Moisture");
  board.registerActor ("heat", 5);
  board.registerActor ("light",6);
  board.registerActor ("bubler",4);
  board.init();
  */
  timeout = millis();
  while (!SI1145.Begin()) {                         //check sunlight sensor
    Serial.println("Si1145 is not ready!");
    delay(1000);
    if (millis() >= timeout + 3000) {               //if sensor is not responding in 3 seconds we consider he is dead
      break;
    }
  }
  if (SI1145.Begin() == 1) {
    Serial.println("Si1145 is ready!");             //sensor is good
  }
  else {
    Serial.println("SI1145 is DEAD!!!");            //sensor is dead
  }

  sensorTemp.begin();

  myBME280.settings.commInterface = I2C_MODE;      //setup & check BME280
  myBME280.settings.I2CAddress = 0x76;
  myBME280.settings.runMode = 3; //  3, Normal mode
  myBME280.settings.tStandby = 101; //  0, 0.5ms
  myBME280.settings.filter = 0; //  0, filter off
  myBME280.settings.tempOverSample = 1;
  myBME280.settings.pressOverSample = 1;
  myBME280.settings.humidOverSample = 1;
  myBME280.begin();

  autoWIFI();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  rtcINIT();
  sendMessage();                                    //Send Config msg 

  //rtc.init();                                       //initialise DS1337
  //rtc.clearAlarm();                                 //clear alarms
  showInfo();                                       //show RTC Informations

  //                                                //initialise light
  int b = rtc.getDate().getHour();
  int c = rtc.getDate().getMinutes();
  Serial.println("Actual Hour : " + String(b + "h" + c));


  if (client.connected()) { 
    client.subscribe(topicDelta);
    Serial.println("Connected to Topic Delta");
  }
  else {//connect to the broker
    reconnect();
  }

  readSensors();                                    //read all the sensors put result to "msg"
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish(topicUpdate, msg);                 //publish message
  //msg[] = " ";

  timeout = millis();
  while (talkback == false) {                                         //wait till you got the feedback
    client.loop();
    if (millis() > timeout + 3000) {
      break;
    }
  }

  Serial.println();
  for (int j = 0; j <= 3; j++) {                      //initialise time for functions they will be compared to the laps time
    timef[j] = millis();                              //0 for Heater, 1 for Light, 2 for Mist and 3 Fans
  }
  timeout = millis();                                 //this timer is used for periodically trigger the readsensors function
}

void loop() {
  laps = millis();                                    //get laps for this loop
  if (MANUAL == 0) {
    schedActors();                                    // Manage On / off Actors schedule dependending on Timer Now.
  }

  while (talkback == false) {                        //wait till you got the feedback
    client.loop();
    yield();
    if (millis() > timeout + 500) {
      break;
    }
  }
  if (timeout + (INTERVAL * 1000) <= laps) {
    Serial.println("report every" + String(INTERVAL * 1000));
    readSensors();                            //read all the sensors and prepare message
    timeout = millis();
    sendMessage();
    //board.update();
  }
}

void schedActors() {
  boolean toHeat = false;
  int nowHour = rtc.getDate().getHour();
  int nowMinute = rtc.getDate().getMinutes();
  
  laps = millis();                                                    //get laps for this loop
    
  if (MANUAL == 0) {
    if (timef[2] + 5000 <= laps) {                                   //once every 10 seconds we check Heater and Mist
      Serial.println("Time Now : "+String(nowHour)+"h"+String(nowMinute));
      Serial.println("Config Sched ----------" );
      Serial.println("Light  - ON/OFF "+ String(LightONhour)  + "H" + String(LightONminute)   + " / " + String(LightOFFhour)  + "H" + String(LightOFFminute) );
      Serial.println("Heat   - ON/OFF "+ String(HeatONhour)   + "H" + String(HeatONminute)    + " / " + String(HeatOFFhour)   + "H" + String(HeatOFFminute) );
      Serial.println("Bubler - ON/OFF "+ String(BublerONhour) + "H" + String(BublerONminute)  + " / " + String(BublerOFFhour) + "H" + String(BublerOFFminute) );
      
      // Checking Thermostat... We do action if Temperature <= HeatTARGET
      sensorTemp.requestTemperatures(); // Send the command to get temperatures
      Temperature1 = sensorTemp.getTempCByIndex(0);
      delay(50);
      /* Thermostat Check */
      if (Temperature1 <= HeatTARGET ) { // && bitRead(Action, pinHeat) == 0) {
        toHeat = true;
        Serial.println("Thermostat Check : Heater ON");
      }
      if (Temperature1 >= HeatTARGET ) { //&& bitRead(Action, pinHeat) == 1) {
        toHeat = false;
        Serial.println("Thermostat Check : Heater OFF");
      }
    
    /* ---------------------- Heat Check ---------------------- */
    if (nowHour > HeatONhour && nowHour < HeatOFFhour && toHeat ) { //needs to be changed if Heat is on over midnight
      doAction(pinHeat, 1);
      Serial.print("Heat ON, Condition 1 / ");
      onHeat = true;      
    }
    else if (nowHour == HeatOFFhour && nowMinute <= HeatOFFminute  && toHeat) { //needs to be changed if Heat is on over midnight
      doAction(pinHeat, 1);
      Serial.print("Heat ON, Condition 2 / ");
      onHeat = true;
    }
    else if (nowHour >= HeatONhour && nowHour < HeatOFFhour && nowMinute >= HeatONminute && toHeat) { //needs to be changed if Heat is on over midnight
      doAction(pinHeat, 1);
      Serial.print("Heat ON, Condition 3 / ");
      onHeat = true;
    }
    else {
      doAction(pinHeat, 0);
      Serial.print("Heat OFF / ");
      onHeat = false;
    }
    
    timef[2] = laps;  
    delay(50);
    
    /* ---------------------- Light Check ---------------------- */
    if (nowHour < LightOFFhour && nowHour > LightONhour ) { //needs to be changed if light is on over midnight
      doAction(pinLight, 1);
      onLight = true;
      tcs.setInterrupt(false);                  // TCS Led On
      Serial.print("Light ON, Condition 1 / ");
    }
    else if (nowHour == LightOFFhour && nowMinute <= LightOFFminute ) { //needs to be changed if light is on over midnight
      doAction(pinLight, 1);
      onLight = true;
      tcs.setInterrupt(false);                  // TCS Led On
      Serial.print("Light ON, Condition 2 / ");
    }
    else if (nowHour >= LightONhour && nowHour < LightOFFhour && nowMinute >= LightONminute) { //needs to be changed if Heat is on over midnight
      doAction(pinLight, 1);
      onLight = true;
      tcs.setInterrupt(false);                  // TCS Led On
      Serial.print("Light ON, Condition 3 / ");
    }
    else {
      doAction(pinLight, 0);
      onLight = false;
      tcs.setInterrupt(true);                  // TCS Led Off
      Serial.print("Light OFF / ");
    }
    delay(50);

    /* ---------------------- Bubler Check ---------------------- */
    if (nowHour < BublerOFFhour && nowHour > BublerONhour ) { //needs to be changed if Bubler is on over midnight
      doAction(pinBubler, 1);
      onBubler = true;
      Serial.println("Bubler ON, Condition 1 / ");
    }
    else if (nowHour == BublerOFFhour && nowMinute <= BublerOFFminute ) { //needs to be changed if Bubler is on over midnight
      doAction(pinBubler, 1);
      onBubler = true;
      Serial.println("Bubler ON, Condition 2 / ");
    }
    else if (nowHour == BublerONhour && nowMinute >= BublerONminute ) { //needs to be changed if Bubler is on over midnight
      doAction(pinBubler, 1);
      onBubler = true;
      Serial.println("Bubler ON, Condition 3 / ");
    }
    else {
      doAction(pinBubler, 0);
      Serial.println("Bubler OFF / ");
      onBubler = false;
    }
    delay(50);
      }
    }
}
