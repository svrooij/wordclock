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
uint32_t yellow = pixels.Color(255, 204, 0);
uint32_t black = pixels.Color(0, 0, 0);

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
  ledsToBlack();
  int ledsToLight = dt.Minute() % 15;

  for(int i=0;i<ledsToLight;i++){
    pixels.setPixelColor(i,yellow);
  }
  pixels.show();
}

void ledsToBlack() {
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i,black);
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



