//// ESP8266 Clock controller
#include <Time.h>
#include <TimeLib.h>

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

// colors
uint32_t colorWhite = pixels.Color(255, 255, 255);
uint32_t colorBlack = pixels.Color(0, 0, 0);
uint32_t colorRed = pixels.Color(255, 0, 0);
uint32_t colorGreen = pixels.Color(0, 255, 0);
uint32_t colorBlue = pixels.Color(0, 0, 255);
uint32_t colorJGreen = pixels.Color(50, 179, 30);

// words (in dutch, sorry) consisting of pixel numbers starting by 0.
// -1 is the marker to know to break the loop.
// Row 1 left to right 0-12
int wrdHetIs[] = {1,2,3,5,6,-1};
//int wrdIs[] = {5,6,-1};
int wrdVijfAlt[] = {8,9,10,11,-1};
// Row 2 right to left 13-23
int wrdTienAlt[] = {23,22,21,20,-1};
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

// Settings
const bool drawHetIs = true;


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


  pixels.begin();
  pixels.setBrightness(10);
  colorWipe(colorRed);
  pixels.show();

  setupClock();
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
  delay(10000);
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
    Serial.print(datestring);
}

// This is the main function for setting the correct words based on the time.
void setLedsByTime(const RtcDateTime& dt){
  colorWipe(colorBlack);
  int minutes = dt.Minute();

  if(drawHetIs){
    paintWord(wrdHetIs);
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
    pixels.show();
  }
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

// Paint a word with default color.
void paintWord(int arrWord[]){
  paintWord(arrWord,colorWhite);
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
