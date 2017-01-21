// ESP8266 Clock controller

// FROM DS1307_Simple
#include <pgmspace.h>
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS1307.h>
RtcDS1307<TwoWire> Rtc(Wire);

// From NeoPixel simple sample
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define LEDPIN 3
#define NUMPIXELS 16
// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

// colors
uint32_t colorWhite = grid.Color(255, 255, 255);
uint32_t colorBlack = grid.Color(0, 0, 0);
uint32_t colorRed = grid.Color(255, 0, 0);
uint32_t colorGreen = grid.Color(0, 255, 0);
uint32_t colorBlue = grid.Color(0, 0, 255);
uint32_t colorJGreen = grid.Color(50, 179, 30);

// words (in dutch, sorry) consisting of pixel numbers starting by 0.
// -1 is the marker to know to break the loop.
// Row 1 left to right 0-12
int wrdHet[] = {1,2,3,-1};
int wrdIs[] = {5,6,-1};
int wrdVijfAlt[] = {8,9,10,11,-1};
// Row 2 right to left 13-23
int wrdTienAlt[] = {23,22,21,20,-1}
int wrdVoor1[] = {16,15,14,13,-1};
// Row 3 left to right 24-34
int wrdOver1[] = {24,25,26,27,-1};
int wrdKwart[] = {30,31,32,33,34,-1};
// Row 4 right to left 35-45
int wrdHalf[] = {45,44,43,42,-1};
int wrdOver2[]= {38,37,36,35,-1};
// Row 5 left to right 46-56
int wrdVoor2[] = {46,47,48,49,-1};
int wrdEen[] = {53,54,55,-1};
// Row 6 right to left 57-67
int wrdTwee [] = {67,66,65,64,-1};
int wrdDrie[] = {60,59,58,57,-1};
// Row 7 left to right 68-78
int wrdVier[] = {68,69,70,71,-1};
int wrdVijf[] = {72,73,74,75,-1};
int wrdZes[] = {76,77,78,-1};
// Row 8 right to left 79-89
int wrdZeven[] = {89,88,87,86,85,-1};
int wrdNegen[] = {83,82,81,80,79,-1};
// Row 9 left to right 90-100
int wrdAcht[] = {90,91,92,93,-1};
int wrdTien[] = {94,95,96,97,-1};
int wrdElf[] = {98,99,100,-1};
// Row 10 right to left 101-113
int wrdTwaalf[] = {112,111,110,109,108,107,-1};
int wrdUur[] = {104,103,102,-1};

int minLeds[] = {0,12,101,113};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);

  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  setupClock();
  pixels.begin();
  pixels.setBrightness(100);
  pixels.show();
}

void setupClock() {
  //--------RTC SETUP ------------
  Rtc.Begin();

  // if you are using ESP-01 then uncomment the line below to reset the pins to
  // the available pins for SDA, SCL
  // Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
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
  // put your main code here, to run repeatedly:
  RtcDateTime now = Rtc.GetDateTime();
  setLedsBasedOnClock(now);
  delay(10000);
}

void setLedsBasedOnClock(const RtcDateTime& dt) {
  colorWipe(colorBlack);
  int ledsToLight = dt.Minute() % 15;

  for(int i=0;i<ledsToLight;i++){
    pixels.setPixelColor(i,yellow);
  }
  pixels.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t color) {
  for(uint16_t i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, color);
  }
  pixels.show();
}

// Paint a word, accepts an array of pixel numbers and a color.
void paintWord(int arrWord[], uint32_t intColor){
  for(int i = 0; i < NUMPIXELS; i++){
    if(arrWord[i] == -1){
      pixels.show();
      break;
    }else{
      pixels.setPixelColor(arrWord[i],intColor);
    }
  }
}

// Same as paintWord, but wait 500ms between each letter.
void spellWord(int arrWord[], uint32_t intColor){
  for(int i = 0; i < NUMPIXELS; i++){
    if(arrWord[i] == -1){
      break;
    }else{
      pixels.setPixelColor(arrWord[i],intColor);
      pixels.show();
      delay(500);
    }
  }
}

#define countof(a) (sizeof(a) / sizeof(a[0]))
void printDateTime(const RtcDateTime& dt) {
    char datestring[20];

    snprintf_P(datestring,
            countof(datestring),
            PSTR("%02u-%02u-%04u %02u:%02u:%02u"),
            dt.Day(),
            dt.Month(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.println(datestring);
}
