#include "Adafruit_NeoPixel.h"
#include "TM1637Display.h"
#include "WiFiManager.h"
#include "NTPClient.h"
#include "DFRobotDFPlayerMini.h"


#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))  // Using a soft serial port
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/16, /*tx =*/17);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif

#define PIN      5  // Strip led DIN
#define red_CLK  21 // changed to pin 21 instead pin 16 for dfplayer use
#define red1_DIO 22 // changed to pin 22 instead 17 for dfplayer use
#define red2_DIO 18
#define red3_DIO 19

#define AM 32
#define PM 33

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 36 // Popular NeoPixel ring size
#define PIXELROWS 12
bool res;
int var=-1;
int analogPin = 34;
const long utcOffsetInSeconds = 3600; // Offset in second
int Hour = 0;
//========================USEFUL VARIABLES=============================
int UTC = 1; // UTC + value in hour -  2 = Summer time
int Display_backlight = 3; // Set displays brightness 0 to 7;
int gmtOffset_sec = 3600;
int daylightOffset_sec = 3600;
int updateInterval = 60000;
//======================================================================

// When setting up the NeoPixel library, we tell it how many pixels,
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
// Setup the red displays
TM1637Display red1(red_CLK, red1_DIO);
TM1637Display red2(red_CLK, red2_DIO);
TM1637Display red3(red_CLK, red3_DIO);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds*UTC, updateInterval);

DFRobotDFPlayerMini myDFPlayer;  // declare mp3 player

void colorOne() {
  for(int i=0; i<12; i++){ pixels.setPixelColor(i, pixels.Color(255,0,0)); }
  for(int i=12; i<24; i++){ pixels.setPixelColor(i, pixels.Color(160,160,0)); }
  for(int i=24; i<NUMPIXELS; i++){ pixels.setPixelColor(i, pixels.Color(255,0,0)); }
  pixels.show();
}

void colorTwo() {
  for(int i=0; i<12; i++) { pixels.setPixelColor(i, pixels.Color(0,0,255)); }
  for(int i=12; i<24; i++) { pixels.setPixelColor(i, pixels.Color(200,250,255)); }
  for(int i=24; i<NUMPIXELS; i++){ pixels.setPixelColor(i, pixels.Color(0,0,255)); }
  pixels.show();
}

void clearColor() {
  pixels.clear();
  for(int i=0; i<NUMPIXELS;i++){ pixels.setPixelColor(i, pixels.Color(0,0,0)); }
  pixels.show();
}

void setup() {
  // dfplayer
  #if (defined ESP32)
    FPSerial.begin(9600, SERIAL_8N1, /*rx =*/16, /*tx =*/17);  // attached pins to dfplayer
  #else
    FPSerial.begin(9600);
  #endif
  pinMode(PIN, OUTPUT);
  pinMode(red_CLK, OUTPUT);
  pinMode(red1_DIO, OUTPUT);
  pinMode(red2_DIO, OUTPUT);
  pinMode(red3_DIO, OUTPUT);
  pinMode(AM, OUTPUT);
  pinMode(PM, OUTPUT);

  pinMode(analogPin, INPUT);

  Serial.begin(9600);
  
  WiFiManager manager;    
     
  // manager.resetSettings(); // Uncomment to reset Wifi
  manager.setTimeout(180);
  //fetches ssid and password and tries to connect, if connections succeeds it starts an access point with the name called "BTTF_CLOCK" and waits in a blocking loop for configuration
  res = manager.autoConnect("BTTF_LAMP_CLOCK","password");
  
  if(!res) {
  Serial.println("failed to connect and timeout occurred");
  ESP.restart(); //reset and try again
  }
  
  delay(2000);

  timeClient.begin();
  red1.setBrightness(Display_backlight);
  red2.setBrightness(Display_backlight);
  red3.setBrightness(Display_backlight);
  pixels.setBrightness(250);
  
  // DFPlayer setup
  if (!myDFPlayer.begin(FPSerial, /*isACK = */ true, /*doReset = */ true)) {  //Use serial to communicate with mp3.
    while (true) {
      delay(0);  // Code to compatible with ESP8266 watch dog.
    }
  }
  myDFPlayer.volume(20);  //Set volume value. From 0 to 30
  myDFPlayer.play(6);     //Play the first mp3/wav
}

void loop() {

  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int currentYear = ptm->tm_year+1900;
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;

  
red1.showNumberDecEx(monthDay,0b01000000,true,2,0);
red1.showNumberDecEx(currentMonth,0b01000000,true,2,2);
red2.showNumberDecEx(currentYear,0b00000000,true);
red3.showNumberDecEx(timeClient.getHours(),0b01000000,true,2,0);
red3.showNumberDecEx(timeClient.getMinutes(),0b01000000,true,2,2);

if((currentMonth*30 + monthDay) >= 121 && (currentMonth*30 + monthDay) < 331){
timeClient.setTimeOffset(utcOffsetInSeconds*UTC);} // Change daylight saving time - Summer
else {timeClient.setTimeOffset((utcOffsetInSeconds*UTC) - 3600);} // Change daylight saving time - Winter


if(timeClient.getHours()>=13){
  digitalWrite(AM,0);
  digitalWrite(PM,1);}
  
else if(timeClient.getHours()==12){
  digitalWrite(AM,0);
  digitalWrite(PM,1);}

else{
  digitalWrite(AM,1);
  digitalWrite(PM,0);}


  pixels.clear(); // Set all pixel colors to 'off'
  
  if(var>15){var = -1;} // Reset counter

// read the switch
  if(analogRead(analogPin) > 512)
  { 
    var = var+1 ;
    myDFPlayer.next();  // Play next mp3/wav on button press
    delay(45);
  }

  delay(100);

  switch (var) {
  case 0: 
    for(int i=0; i<7;i++){ pixels.setPixelColor(i, pixels.Color(255,0,0)); }
    for(int i=7; i<13;i++){ pixels.setPixelColor(i, pixels.Color(160,160,0)); }
    for(int i=13; i<19;i++){ pixels.setPixelColor(i, pixels.Color(255,0,0)); }
    for(int i=19; i<25;i++){ pixels.setPixelColor(i, pixels.Color(255,0,0)); }
    for(int i=25; i<31;i++){ pixels.setPixelColor(i, pixels.Color(255,0,0)); }
    for(int i=31; i<37;i++){ pixels.setPixelColor(i, pixels.Color(160,160,0)); }
    pixels.show();
    break;

  case 1:
    clearColor();
    break;

  case 2:
    for(int i=0; i<7;i++){ pixels.setPixelColor(i, pixels.Color(0,0,255)); }
    for(int i=7; i<13;i++){ pixels.setPixelColor(i, pixels.Color(200,250,255)); }
    for(int i=13; i<19;i++){ pixels.setPixelColor(i, pixels.Color(0,0,255)); }
    for(int i=19; i<25;i++){ pixels.setPixelColor(i, pixels.Color(0,0,255)); }
    for(int i=25; i<31;i++){ pixels.setPixelColor(i, pixels.Color(200,250,255)); }
    for(int i=31; i<37;i++){ pixels.setPixelColor(i, pixels.Color(0,0,255)); }
    pixels.show();
    break;

  case 3:
    clearColor();
    break;

  case 4:
    pixels.clear();
    for(int i=0; i<7;i++){ pixels.setPixelColor(i, pixels.Color(255,0,10)); }
    for(int i=7; i<13;i++){pixels.setPixelColor(i, pixels.Color(0,10,255)); }
    for(int i=13; i<19;i++){ pixels.setPixelColor(i, pixels.Color(255,0,10)); }
    for(int i=19; i<25;i++){ pixels.setPixelColor(i, pixels.Color(255,0,10)); }
    for(int i=25; i<31;i++){ pixels.setPixelColor(i, pixels.Color(0,10,255)); }
    for(int i=31; i<37;i++){ pixels.setPixelColor(i, pixels.Color(255,0,10)); }
    pixels.show();
    break;

  case 5:
    clearColor();
    break;

  case 6:
    pixels.clear();
    for (int i = 0; i < PIXELROWS; i++) { pixels.setPixelColor(i, pixels.Color(255, 99, 71)); }
    for (int i = PIXELROWS; i < (PIXELROWS * 2); i++) { pixels.setPixelColor(i, pixels.Color(127, 0, 255)); }
    for (int i = (PIXELROWS * 2); i < NUMPIXELS; i++) { pixels.setPixelColor(i, pixels.Color(255, 99, 71)); }
    pixels.show();
    break;

  case 7:
    clearColor();
    break;

  case 8:
    pixels.clear();
    for (int i = 0; i < PIXELROWS; i++) { pixels.setPixelColor(i, pixels.Color(124, 252, 0)); }
    for (int i = PIXELROWS; i < (PIXELROWS * 2); i++) { pixels.setPixelColor(i, pixels.Color(255, 150, 0)); }
    for (int i = (PIXELROWS * 2); i < NUMPIXELS; i++) { pixels.setPixelColor(i, pixels.Color(124, 252, 0)); }
    pixels.show();
    break;

  case 9:
    clearColor();
    break;

  case 10:
    pixels.clear();
    for (int i = 0; i < PIXELROWS; i++) { pixels.setPixelColor(i, pixels.Color(70, 0, 150)); }
    for (int i = PIXELROWS; i < (PIXELROWS * 2); i++) { pixels.setPixelColor(i, pixels.Color(170, 100, 255)); }
    for (int i = (PIXELROWS * 2); i < NUMPIXELS; i++) { pixels.setPixelColor(i, pixels.Color(70, 0, 150)); }
    pixels.show();
    break;

  case 11:
    clearColor();
    break;

  case 12:
    colorOne();
    break;

  case 13:
    clearColor();
    break;

  case 14:
    colorTwo();
    break;
 
  case 15:
    clearColor();
    break;

  default:
    colorOne();
    break;
  }
}
