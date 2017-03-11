// -------------------------------------
// Dutch Wordclock by Stephan van Rooij
//
// github@svrooij.nl
//
// Hardware:
// - ESP8266
// - DS1307 (TinyRTC module)
// - 114x WS2811
// -------------------------------------
//// ESP8266 Clock controller
#include <Time.h>

#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

/* for normal hardware wire use below */
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS1307.h>
RtcDS1307<TwoWire> Rtc(Wire);
/* for normal hardware wire use above */

// From NeoPixel simple sample
#include <Adafruit_NeoPixel.h>

#define LEDPIN 3
#define NUMPIXELS 114

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

// Some colors (not all are used at the moment)
uint32_t colorWhite = pixels.Color(255, 255, 255);
uint32_t colorBlack = pixels.Color(0, 0, 0);
uint32_t colorRed = pixels.Color(255, 0, 0);
uint32_t colorGreen = pixels.Color(0, 255, 0);
uint32_t colorBlue = pixels.Color(0, 0, 255);
uint32_t colorJGreen = pixels.Color(50, 179, 30);
uint32_t colorYellow = pixels.Color(255,255, 0);

// words (in dutch, sorry) consisting of pixel numbers starting by 0.
// -1 is the marker to know to break the loop.
// Row 1 left to right 0-12
// H E T K I S A V I J F
int wrdHetIs[] = {1,2,3,5,6,-1};
//int wrdIs[] = {5,6,-1};
int wrdVijfAlt[] = {8,9,10,11,-1};
// Row 2 right to left 13-23
// T I E N A T Z V O O R
int wrdTienAlt[] = {23,22,21,20,-1};
int wrdVoor1[] = {16,15,14,13,-1};
// Row 3 left to right 24-34
// O V E R M E K W A R T
int wrdOver1[] = {24,25,26,27,-1};
int wrdKwart[] = {30,31,32,33,34,-1};
// Row 4 right to left 35-45
// H A L F S P M O V E R
int wrdHalf[] = {45,44,43,42,-1};
int wrdOver2[]= {38,37,36,35,-1};
// Row 5 left to right 46-56
// V O O R T H G E E N S
int wrdVoor2[] = {46,47,48,49,-1};
int wrdEen[] = {53,54,55,-1};
// Row 6 right to left 57-67
// T W E E A M C D R I E
int wrdTwee [] = {67,66,65,64,-1};
int wrdDrie[] = {60,59,58,57,-1};
// Row 7 left to right 68-78
// V I E R V I J F Z E S
int wrdVier[] = {68,69,70,71,-1};
int wrdVijf[] = {72,73,74,75,-1};
int wrdZes[] = {76,77,78,-1};
// Row 8 right to left 79-89
// Z E V E N O N E G E N
int wrdZeven[] = {89,88,87,86,85,-1};
int wrdNegen[] = {83,82,81,80,79,-1};
// Row 9 left to right 90-100
// A C H T T I E N E L F
int wrdAcht[] = {90,91,92,93,-1};
int wrdTien[] = {94,95,96,97,-1};
int wrdElf[] = {98,99,100,-1};
// Row 10 right to left 101-113
// T W A A L F P M U U R
int wrdTwaalf[] = {112,111,110,109,108,107,-1};
int wrdUur[] = {104,103,102,-1};

// Leds used for minutes between the words.
int minLeds[] = {0,12,101,113};

// Extra words
int wrdStephan[] = {6,18,29,40,51,63,85,-1};

// Settings
const bool drawHetIs = true;

// ---------------------------------------------
// Arduino OTA (BasicOTA sample)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Input your own SSID and password here for OTA updates
const char* ssid = "wifi-name";
const char* password = "wifipassword";

void setupOTA(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start OTA");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd OTA");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress OTA: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}
// Arduino OTA (BasicOTA sample)
// ---------------------------------------------

void setup() {
  // Set the correct pins for I2c
  //Wire.begin(int sda, int scl);
  Wire.begin(D2,D1);

  // put your setup code here, to run once:
  Serial.begin(230400);
//
//  Serial.print("compiled: ");
//  Serial.print(__DATE__);
//  Serial.println(__TIME__);
  delay(1000);

  // Configure the Clock module.
  setupClock();

  // Remove this line to disable OTA
  setupOTA();

  // Start neopixel module and run some tests.
  pixels.begin();
  testLeds();
}

// Setup for the RTC module.
// Time is set to compile time when not running.
void setupClock() {
  // Start
  Rtc.Begin();

  // Set a date to be used for checking and setting the RTC.
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  // Print the compiled datetime to console
  printDateTime(compiled);

  Serial.println();

  if (!Rtc.IsDateTimeValid())
  {
      // Common Cuases:
      //    1) first time you ran and the device wasn't running yet
      //    2) the battery on the device is low or even missing

      Serial.println("RTC lost confidence in the DateTime!");

      // following line sets the RTC to the date & time this sketch was compiled
      // it will also reset the valid flag internally unless the Rtc device is
      // having an issue

      Rtc.SetDateTime(compiled);
  }

  if (!Rtc.GetIsRunning())
  {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
      Serial.println("RTC is older than compile time!  (Updating DateTime)");
      Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
      Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
      Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.SetSquareWavePin(DS1307SquareWaveOut_Low);
}

void loop() {
  if(Rtc.IsDateTimeValid()){
    RtcDateTime now = Rtc.GetDateTime();

    printDateTime(now);
    setLedsByTime(now);

  } else {
    // Should the app crash if the time is not correct??
  }
  ArduinoOTA.handle();
  delay(5000);
}

// function copied from rtc sample.
#define countof(a) (sizeof(a) / sizeof(a[0]))

// Print the DateTime to the console (only for debugging).
void printDateTime(const RtcDateTime& dt) {
    char datestring[20];

    snprintf_P(datestring,
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.println(datestring);
}

void testLeds(){

  pixels.setBrightness(40);
  colorWipe(colorRed,30);
  colorWipe(colorGreen,30);
  colorWipe(colorBlue,30);
  colorWipe(colorBlack,30);
  spellWord(wrdStephan,colorJGreen);
  delay(3000);
}

// This is the main function for setting the correct words based on the time.
void setLedsByTime(const RtcDateTime& dt){
  colorWipe(colorBlack);
  
  int minutes = dt.Minute();

  if(minutes == 0 && dt.Second() < 11){
    spellWord(wrdStephan,colorJGreen);
    delay(9000);
    colorWipe(colorBlack);
  }

  int uren = dt.Hour();
  if(uren >= 8 && uren < 22) {
    pixels.setBrightness(100);
  } else {
    pixels.setBrightness(40);
  }

  if(drawHetIs){
    paintWord(wrdHetIs,colorWhite);
  }

  // modMin is the minutes left over if devived by 5
  int modMin = minutes % 5;

  if(minutes > 5) {
    // The words only change every 5 minutes
    // So we take the minutes minus the modulo 5 and devide it by 5.
    int dividedMin = (minutes - modMin) / 5;
    paintMinuteWords(dividedMin);
  }

  // Hour setting.
  // We only need 12 hours, hence the modulo.
  // When the minutes are above 19 we start calling it .. before the next hour.
  int uur = dt.Hour() % 12;
  if(minutes > 19) {
    uur++;
  }
  paintHour(uur);

  // Uur
  if(minutes >= 0 && minutes < 6){
    paintWord(wrdUur);
  }

  // Minute LEDS in the 4 corners
  if(modMin > 0) {
    for(int i = 0; i < modMin;i++){
      pixels.setPixelColor(minLeds[i],colorWhite);
    }

  }

  pixels.show();
}

// Paint the words in between hours.
void paintMinuteWords(int devMin){
  switch(devMin){
    case 1:
      paintWord(wrdVijfAlt);
      paintWord(wrdOver1);
      break;
    case 2:
      paintWord(wrdTienAlt);
      paintWord(wrdOver2);
      break;
    case 3:
      paintWord(wrdKwart);
      paintWord(wrdOver2);
      break;
    case 4:
      paintWord(wrdTienAlt);
      paintWord(wrdVoor1);
      paintWord(wrdHalf);
      break;
    case 5:
      paintWord(wrdVijfAlt);
      paintWord(wrdVoor1);
      paintWord(wrdHalf);
      break;
    case 6:
      paintWord(wrdHalf);
      break;
    case 7:
      paintWord(wrdVijfAlt);
      paintWord(wrdOver1);
      paintWord(wrdHalf);
      break;
    case 8:
      paintWord(wrdTienAlt);
      paintWord(wrdOver1);
      paintWord(wrdHalf);
      break;
    case 9:
      paintWord(wrdKwart);
      paintWord(wrdVoor2);
      break;
    case 10:
      paintWord(wrdTienAlt);
      paintWord(wrdVoor2); // maybe other one? based on hour?
      break;
    case 11:
      paintWord(wrdVijfAlt);
      paintWord(wrdVoor2);
      break;
  }

}

// Paint the correct word by hour number.
void paintHour(int hours){
  switch(hours){
    case 1:
      paintWord(wrdEen);
      break;
    case 2:
      paintWord(wrdTwee);
      break;
    case 3:
      paintWord(wrdDrie);
      break;
    case 4:
      paintWord(wrdVier);
      break;
    case 5:
      paintWord(wrdVijf);
      break;
    case 6:
      paintWord(wrdZes);
      break;
    case 7:
      paintWord(wrdZeven);
      break;
    case 8:
      paintWord(wrdAcht);
      break;
    case 9:
      paintWord(wrdNegen);
      break;
    case 10:
      paintWord(wrdTien);
      break;
    case 11:
      paintWord(wrdElf);
      break;
    case 12:
    case 0:
      paintWord(wrdTwaalf);
      break;
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t color) {
  for(uint16_t i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, color);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t color, int wait) {
  for(uint16_t i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, color);
      if(wait > 0){
        pixels.show();
        delay(wait);
      }
  }
}

// Paint a word, accepts an array of pixel numbers and a color.
void paintWord(int arrWord[], uint32_t intColor){
  for(int i = 0; i < NUMPIXELS; i++){
    if(arrWord[i] == -1){
      //pixels.show();
      //pixels.show();
      break;
    }else{
      pixels.setPixelColor(arrWord[i],intColor);
    }
  }
}

// Paint a word with default color.
void paintWord(int arrWord[]){
  paintWord(arrWord,colorWhite);
}

// Same as paintWord, but wait 300ms between each letter.
void spellWord(int arrWord[], uint32_t intColor){
  for(int i = 0; i < NUMPIXELS; i++){
    if(arrWord[i] == -1){
      break;
    }else{
      pixels.setPixelColor(arrWord[i],intColor);
      pixels.show();
      delay(300);
    }
  }
}
