//////////////////////////////////////////////////////////////////////
//==================================================================//
//                           PRE-SETUP                              //
//==================================================================//
//////////////////////////////////////////////////////////////////////

//====================================================================
// LIBRARIES (included within the COOL_Board) ========================
//====================================================================
// HARDWARE ==========================================================
#include "Arduino.h"                                                        // Necessary to run external board with Arduino IDE & functions
#include <DS1337.h>                                                         // Necessary to run the Real-time Clock onboard
#include <NeoPixelBus.h>                                                    // Necessary to play with NeoPixel LED onboard
#include <Wire.h>                                                           // Necessary to communicate in I2C protocol (most of COOL sensors)
// SENSORS ===========================================================
#include "SI114X.h"                                                         // Necessary to run Light sensor onboard (Si1145)
#include "SparkFunBME280.h"                                                 // Necessary to run Temp & Hum & Pressue sensor onboard (BME280)
#include <stdint.h>                                                         // Necessary to run "SparkFunBME280.h"
// WIFI ==============================================================
#include <ESP8266WiFi.h>                                                    // Necessary to connect the COOL Board on wifi network
#include <ESP8266WebServer.h>                                               // Necessary to run onboard server
#include <PubSubClient.h>                                                   // Necessary to communicate as a client with MQTT Brocker
// CONTENTS ==========================================================
#include "FS.h"                                                             // Necessary to organize and access files with the onboard server
#include "QuickStats.h"                                                     // Necessary to do math with statistics (average)
#include <ArduinoJson.h>                                                    // Necessary to play with json files and Arduino
#include <WiFiUdp.h>
//====================================================================
// OBJECTS ===========================================================
//====================================================================
SI114X SI1145 = SI114X();
BME280 mySensor1;
WiFiClient espClient;
PubSubClient client(espClient);
WiFiUDP udp;
DS1337 rtc;
QuickStats stats;
//====================================================================
// SETTINGS (some specific objects) ==================================
//====================================================================
// LED ===============================================================
#define colorSaturation 128
const uint16_t PixelCount = 1;                                              // this example assumes 4 pixels, making it smaller will cause a failure
const uint8_t PixelPin = 2;                                                 // make sure to set this to the correct pin, ignored for Esp8266
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

//////////////////////////////////////////////////////////////////////
//==================================================================//
//                           VARIABLES                              //
//==================================================================//
//////////////////////////////////////////////////////////////////////
//====================================================================
const bool DEBUG_FLAG = 1;
//====================================================================
//const char* mqtt_server = "52.17.46.139";
const char* mqtt_server = "172.16.10.125";

IPAddress timeServerIP;                                                     // time.nist.gov NTP server address
//====================================================================
const char* ntpServerName = "time.nist.gov";
//====================================================================
const int NTP_PACKET_SIZE = 48;                                             // NTP time stamp is in the first 48 bytes of the message
byte NTPBuffer[ NTP_PACKET_SIZE];                                           //buffer to hold incoming and outgoing NTP packets
//====================================================================
// SENSORS ===========================================================
//====================================================================
// MOISTURE ==========================================================
int EnMoisture = 13;
int Moisture = 0;                                                           // Create a Moisture input value with a "int" (variable name = "Moisture")
// LIGHT (Si1145) ====================================================
int Vis = 0;                                                                // Create a Visible Light input value with a "int" (variable name = "Vis")
float UV = 0;                                                               // Create a UV input value with a "float" (variable name = "UV")
int IR = 0;                                                                 // Create a Moisture input value with a "int" (variable name = "IR")
// TEMP & HUM (BME280) ===============================================
float Temp = 0;                                                             // Create a variable Temperature with an "float" (variable name = "Temp")
float Humi = 0;                                                             // Create a variable Humidity with an "float" (variable name = "Humi")
float Pres = 0;                                                             // Create a variable Pressure with an "float" (variable name = "Pres")
// POWER (LiPo) ======================================================
float Bat = 0;                                                              // Create a Battery Tension input value with a "float" (variable name = "Bat")
//====================================================================
// MQTT MSG FORMAT ===================================================
//====================================================================
char str_UV[16];
char str_Temp[16];
char str_Humi[16];
char str_Pres[16];
char str_Bat[16];
uint8_t MAC_array[6];
char MAC_char[16];
char str_Sig[4];
char str_TIME[16];
char str_DATE[16];
char str_wifi[16];
//====================================================================
long lastMsg = 0;
char msg[300];
char delta[300];
int value = 0;
//====================================================================
bool talkback = false;                                                      //config was modified
bool action = false;                                                        //do a action
int AnMplex = 12;
int EnI2C = 5;
int Pump = 15;
int MM = 1;
int PT = 20;
int P = 0;
int INTERVAL = 120;                                                         //Sets wakeup interval in SECONDS in equal 15minutes = 900seconds, 1 hour = 3600seconds
unsigned long timeout;
unsigned long TIMESYNC;                                                     //variable for the last time we updated Time

//////////////////////////////////////////////////////////////////////
//==================================================================//
//                           FUNCTIONS                              //
//==================================================================//
//////////////////////////////////////////////////////////////////////

//====================================================================
// CONFIG FILE =======================================================
//====================================================================
// All presets config from USER to the COOL_Board
// LOAD CONFIG.JSON ==================================================
bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");                       // OPEN A SPECIFIC FILE FROM LOCAL SERVER (within SPIFFS)
  if (!configFile) {                                                        // CHECK 1 : IF THE FILE ISN'T AVAILABLE
    Serial.println("Failed to open config file");
    return false;
  }
  size_t size = configFile.size();
  if (size > 1024) {                                                        // CHECK 2 : IF THE FILE IS TOO BIG
    Serial.println("Config file size is too large");
    return false;
  }
  std::unique_ptr<char[]> buf(new char[size]);                              // CHECK 3 : IF THE FILE IS READABLE Allocate a buffer to store contents of the file.
  configFile.readBytes(buf.get(), size);                                    // We don't use String here because ArduinoJson library requires the input buffer to be mutable. If you don't use ArduinoJson, you may as well use configFile.readString instead.
  StaticJsonBuffer<200> jsonBuffer;                                         // Need to use a buffer to read a json
  JsonObject& json = jsonBuffer.parseObject(buf.get());                     // Run a function to parse jsonBuffer
  if (!json.success()) {                                                    // CHECK 4 : IF THE FILE IS PARSABLE
    Serial.println("Failed to parse config file");
    return false;
  }
  PT = json["PT"];                                                          // GET ALL DATA FROM JSON
  MM = json["MM"];
  INTERVAL = json["INTERVAL"];
  TIMESYNC = json["TIMESYNC"];
  Serial.println();                                                         // PRINT ALL DATA READ
  Serial.println(PT);
  Serial.println(MM);
  Serial.println(INTERVAL);
  Serial.println(TIMESYNC);
  configFile.close();                                                       // CLOSE FILE
}
// SAVE CONFIG.JSON ==================================================
bool saveConfig() {
  SPIFFS.remove("/config.json");                                            // OPEN A SPECIFIC FILE FROM LOCAL SERVER (within SPIFFS)
  StaticJsonBuffer<200> jsonBuffer;                                         // Need to use a buffer to read a json
  JsonObject& json = jsonBuffer.createObject();                             // Run a function to fill jsonBuffer
  json["PT"] = PT;                                                          // POST ALL THE VALUES IN VARIABLES
  json["MM"] = MM;
  json["INTERVAL"] = INTERVAL;
  json["TIMESYNC"] = TIMESYNC;
  File configFile = SPIFFS.open("/config.json", "a+");                      // PUSH INTO THE GOOD FILE
  if (!configFile) {                                                        // CHECK AVAILABILITY
    Serial.println("Failed to open config file for writing");
    return false;
  }
  json.printTo(configFile);                                                 // WRITE ON FILE
  json.printTo(Serial);                                                     // PRINT IT INTO THE SERIAL MONITOR
  configFile.close();                                                       // CLOSE FILE
  return true;
}

//====================================================================
// READ VALUES =======================================================
//====================================================================
// All sensors values are available by specific function
// BATTERY TENSION ===================================================
float readVBat() {
  digitalWrite(AnMplex, LOW);                                               //Enable Analog Switch to get the Batterie tension
  delay(200);
  int raw = analogRead(A0);                                                 //read in Batterie tension
  //Serial.println(raw);
  float val = 6.04 / 1024 * raw;                                            //convert it apprimatly right tension in volts
  return val;
}
// SOIL MOISTURE =====================================================
int readMoisture() {
  digitalWrite(EnMoisture, LOW);                                            //enable moisture sensor and waith a bit
  digitalWrite(AnMplex, HIGH);
  delay(2000);
  int val = analogRead(A0);                                                 //read the value form the moisture sensor
  digitalWrite(EnMoisture, HIGH);                                           //disable moisture sensor for minimum wear
  return val;
}
// READ ALL VALUES ===================================================
void readSensors() {
  Led(0, 128, 0, 10, 4);                                                    // YOU JUST HAVE TO COMMENT LINE TO SELECT OR DESELECT VALUES TO SEND. FIRST YOU GET THE VALUES FOR EACH VARIABLES YOU WANT TO SEND
  dtostrf(WiFi.RSSI(), 3, 0, str_Sig);
  Bat = readVBat();
  Moisture = readMoisture();
  int P = digitalRead(Pump);
  Vis = SI1145.ReadVisible();
  IR = SI1145.ReadIR();
  UV = (float)SI1145.ReadUV();
  UV = UV / 100;
  Temp = mySensor1.readTempC();
  Pres = mySensor1.readFloatPressure();
  Humi = mySensor1.readFloatHumidity();
  rtc.getDate().getTimeString().toCharArray(str_TIME, 16);
  rtc.getDate().getDateString().toCharArray(str_DATE, 16);
  dtostrf(UV, 4, 2, str_UV);
  dtostrf(Temp, 4, 2, str_Temp);
  dtostrf(Humi, 4, 2, str_Humi);
  dtostrf(Pres, 6, 0, str_Pres);
  dtostrf(Bat, 4, 2, str_Bat);
  // FORMAT A SPECIFIC MSG TO POST IT ON AWS/MQTT BROCKER WITH VARIABLES
  snprintf(msg, 300, "{\"state\":{\"reported\":{\"Id\":\"%s\",\"timestamp\":\"20%sT%sZ\",\"Temp\":%s,\"Humi\":%s,\"Pres\":%s,\"Vis\":%ld,\"IR\":%ld,\"UV\":%s,\"Moist\":%ld,\"Bat\":%s,\"Sig\":%s}}}",
           //MAC_char,
           "TamataRasPi2",
           str_DATE,
           str_TIME,
           str_Temp,
           str_Humi,
           str_Pres,
           Vis,
           IR,
           str_UV,
           Moisture,
           str_Bat,
           str_Sig);
}

//====================================================================
// ACTORS ============================================================
//====================================================================
// RUN ON/OFF A PUMP =================================================
void doPump() {
  if (Moisture <= MM || P == 1) {
    snprintf(msg, 300, "{\"state\":{\"reported\":{\"P\":1},\"desired\":null}}}");
    Serial.println(msg);
    //client.publish("$aws/things/mosquittobroker/shadow/update", msg);
    client.publish("tamataraspi/spiru", msg);
    timeout = millis();
    Serial.print("Pump ON");
    digitalWrite(Pump, HIGH);
    while (millis() < (timeout + PT * 1000)) {
      delay(1000);
    }
    Serial.print("Pump OFF");
    digitalWrite(Pump, LOW);
  }
}

//====================================================================
// WIFI & MQTT FUNCTIONS =============================================
//====================================================================
// AUTO DETECT OLD OR KNOWN WIFI NETWORKS ============================
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
  float bestSig = -300 ;                                      //value for the strongest Wifi Signal

  for (int i = 0; i <= num - 1; i++) {                        //initialize values and read networks & password to the memory
    a.readStringUntil(':');                                   //parsing is done "by hand" since arduinoJSON creates impossible bugs..
    a.readStringUntil('"');
    match[i] = false;                                         //reset all match flags since we need to start from blank
    ssid[i] = a.readStringUntil('"');                         //read SSID in memory
    a.readStringUntil('"');
    password[i] = a.readStringUntil('"');                     //read password into memory
    matchSig[i] = -300;                                       //set a default value below WiFi capabilities of the ESP for Signal strenght detection
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
          matchSig[j] = (float)WiFi.RSSI(k);                  //note Signal strenght
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
// CALLBACK FROM MQTT BROCKER ========================================
void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonBuffer<300> jsonBuffer;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  snprintf(delta, 300, "%s", payload);
  Serial.write(delta);
  Serial.println();
  JsonObject& root = jsonBuffer.parseObject(delta);
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return;
  }
  if (root["state"]["PT"] != 0) {
    PT = root["state"]["PT"];
    talkback = true;
  }
  if (root["state"]["MM"] != 0) {
    MM = root["state"]["MM"];
    talkback = true;
  }
  if (root["state"]["INTERVAL"] != 0) {
    INTERVAL = root["state"]["INTERVAL"];
    talkback = true;
  }
  if (root["state"]["P"] != 0) {
    P = root["state"]["P"];
    action = true;
  }
}
// AUTOCONNECTION WITH MQTT BROCKER WHEN IT'S KNOWN ==================
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MAC_char)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("topic/test", "hello world");
      // ... and resubscribe
      //client.subscribe("$aws/things/mosquittobroker/shadow/update/delta");
      client.subscribe("tamataraspi/delta");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//====================================================================
// RGB LED ANIMATIONS ================================================
//====================================================================
// DEFINE DIFFERENT LED CONFIG =======================================
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
// FADE A COLOR ======================================================
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
// BLINK =============================================================
void OnOff(int R, int G, int B, int T) {
  strip.SetPixelColor(0, RgbColor(R, G, B));
  strip.Show();
  delay(T);
  strip.SetPixelColor(0, RgbColor(0, 0, 0));
  strip.Show();
}
// FADE IN ===========================================================
void FadeIn(int R, int G, int B, int T) {
  for (int k = 0; k < 100; k++) {
    strip.SetPixelColor(0, RgbColor(k * R / 100, k * G / 100, k * B / 100));
    strip.Show();
    delay(T);
  }
}
// FADE OUT ==========================================================
void FadeOut(int R, int G, int B, int T) {
  for (int k = 100; k >= 0; k--) {
    strip.SetPixelColor(0, RgbColor(k * R / 100, k * G / 100, k * B / 100));
    strip.Show();
    delay(T);
  }
}
// STROBOSCOPIC ======================================================
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

//====================================================================
// HARDWARE FUNCTIONS ================================================
//====================================================================
// SLEEP AND WAKEUP ==================================================
void gotosleep() {
  Serial.print("GOTO SLEEP for ");
  Serial.print(INTERVAL / 60);
  Serial.println(" minutes");
  Led(128, 128, 128, 3, 3);
  ESP.deepSleep((INTERVAL * 1000000), WAKE_RF_DEFAULT);                     //set deepsleep periode in seconds
}
// SHOW INFORMATION ==================================================
void showInfo() {
  Serial.println("INFO");
  Serial.println("----");
  Serial.print("CLOCK: ");                                                  // clock
  Serial.print(rtc.getDate().getTimeString());                              // time
  Serial.print(" - ");
  Serial.print(rtc.getDate().getDateString());                              // date
  if (!rtc.isRunning())                                                     // oscillator
    Serial.print(" STOPPED");
  else
    Serial.print(" RUNNING");
  if (rtc.hasStopped())
    Serial.print(" HAS STOPPED");
  Serial.println();
  Serial.print("ALARM: ");                                                  // alarm
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

  int tickMode = rtc.getTickMode();                                         // tick mode
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
  udp.begin(2390);                                                          //some random port for UDP communication
  WiFi.hostByName(ntpServerName, timeServerIP);
  sendNTPpacket(timeServerIP);                                              // send an NTP packet to a time server
  timeout = millis();
  while (!cb) {                                                             //check sunlight sensor
    byte count = 0;
    cb = udp.parsePacket();
    delay(1000);
    if (millis() >= timeout + 3000) {                                       //retry every 3 seconds till receiving a packet
      WiFi.hostByName(ntpServerName, timeServerIP);                         //build hostname
      sendNTPpacket(timeServerIP);                                          // send an NTP packet to a time server
      timeout = millis();
    }
    if (count == 10) {                                                      //if we didn't get a answer after 10 seconds we break
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
unsigned long sendNTPpacket(IPAddress& address){
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


/* -----------------------------------------------------------------*/
/* --------- ARDUINO CODE NOW --------------------------------------*/
/* -----------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
//==================================================================//
//                              SETUP                               //
//==================================================================//
//////////////////////////////////////////////////////////////////////
/* -----------------------------------------------------------------*/
/* --------- ARDUINO SETUP -----------------------------------------*/
/* -----------------------------------------------------------------*/
// SETUP =============================================================
void setup() {
  pinMode(AnMplex, OUTPUT);                                                 //Declare Analog Multiplexer OUTPUT
  pinMode(EnMoisture, OUTPUT);                                              //Declare Moisture enable Pin
  pinMode(EnI2C, OUTPUT);
  pinMode(Pump, OUTPUT);
  digitalWrite(AnMplex, LOW);                                               //put the multiplexer low (VBat)
  digitalWrite(EnMoisture, HIGH);
  digitalWrite(EnI2C, HIGH);
  digitalWrite(Pump, LOW);

  if (DEBUG_FLAG == 0) {                                                    //see if the debug Flag was set
    strip.Begin();                                                          //if not start WS2812 led
    strip.SetPixelColor(0, RgbColor(0, 0, 0));                              //cut the light
    strip.Show();                                                           //push pixel
  }
  else {
    Serial.begin(115200);                                                   //If the debug was set enable Serial
    delay(2000);
  }
  SPIFFS.begin();                                                           //Initialize Filesystem

  loadConfig();                                                             //load parameters from SPIFFS

  Wire.begin(2, 14);                                                        //I2C init

  WiFi.macAddress(MAC_array);                                               //Get mac adresse and store it for later
  for (int i = 0; i < sizeof(MAC_array); ++i) {
    sprintf(MAC_char, "%s%02x", MAC_char, MAC_array[i]);
  }

  timeout = millis();
  while (!SI1145.Begin()) {                                                 //check sunlight sensor
    Serial.println("Si1145 is not ready!");
    delay(1000);
    if (millis() >= timeout + 3000) {                                       //if sensor is not responding in 3 seconds we consider he is dead
      break;
    }
  }
  if (SI1145.Begin() == 1) {
    Serial.println("Si1145 is ready!");                                     //sensor is good
  }
  else {
    Serial.println("SI1145 is DEAD!!!");                                    //sensor is dead
  }

  mySensor1.settings.commInterface = I2C_MODE;                              //setup & check BME280
  mySensor1.settings.I2CAddress = 0x76;
  mySensor1.settings.runMode = 3;                                           //  3, Normal mode
  mySensor1.settings.tStandby = 101;                                         //  0, 0.5ms
  mySensor1.settings.filter = 0;                                            //  0, filter off
  mySensor1.settings.tempOverSample = 1;
  mySensor1.settings.pressOverSample = 1;
  mySensor1.settings.humidOverSample = 1;
  mySensor1.begin();

  autoWIFI();                                                               //Serial.begin(115200);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  rtcINIT();
  showInfo();                                                               //show RTC Informations

  if (!client.connected()) {                                                //connect to the broker
    reconnect();
  }
  readSensors();                                                            //read all the sensors and prepare message
  Serial.print("Publish message: ");
  Serial.println(msg);
  //client.publish("$aws/things/mosquittobroker/shadow/update", msg);         //publish message
  client.publish("tamataraspi/spiru", msg);

  timeout = millis();
  while (talkback == false) {                                               //wait till you got the feedback
    client.loop();
    if (millis() > timeout + 3000) {
      break;
    }
  }

  if (talkback == true) {
    if (!saveConfig()) {
      Serial.println("Failed to save config");
    } else {
      Serial.println("Config saved");
      snprintf(msg, 300, "{\"state\":{\"reported\":{\"PT\": %ld,\"MM\":%ld,\"INTERVAL\":%ld},\"desired\":null}}",
               PT,
               MM,
               INTERVAL);
      Serial.print("Config sent : ");
      Serial.println(msg);
      //client.publish("$aws/things/mosquittobroker/shadow/update", msg);     //publish message
      client.publish("tamataraspi/spiru", msg);
    }
  }

  //gotosleep();                                                              //take a nap
}

//////////////////////////////////////////////////////////////////////
//==================================================================//
//                              LOOP                                //
//==================================================================//
//////////////////////////////////////////////////////////////////////
void loop() {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > 5000) {
    readSensors();
    lastMsg = now;
    Serial.print("Publish message: ");
    Serial.println(msg);
    //client.publish("$aws/things/mosquittobroker/shadow/update", msg);
    client.publish("tamataraspi/spiru",msg);
    
    }
}
