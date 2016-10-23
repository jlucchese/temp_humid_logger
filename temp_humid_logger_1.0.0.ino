String timeNow;
String readLine;

const int ledReadPin = 8;
const int ledRunPin = 9;
int ledRunState = LOW;

// The value will quickly become too large for an int to store
unsigned long previousMillisRead = 0;
unsigned long previousMillisRun = 0;

// constants won't change:
const long intervalRead = 15000;
const long intervalRun = 333;

// ***********
// DHT 22 (TEMPERATURA E UMIDADE)
#include "DHT.h"
#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

// ***********
// DSC1037 (DATA E HORA)
#include <Wire.h>
#include "RTClib.h"

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
#define Serial SerialUSB
#endif

RTC_DS1307 rtc;

// ***********
// SD CARD
#include <SPI.h>
#include <SD.h>

File myFile;

void setup() {
  pinMode(ledReadPin, OUTPUT);
  pinMode(ledRunPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("DHTxx test!");

  dht.begin();

#ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }


}

void loop() {
  digitalWrite(ledReadPin, LOW);

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisRun >= intervalRun) {
    previousMillisRun = currentMillis;
    if (ledRunState == LOW) {
      ledRunState = HIGH;
    } else {
      ledRunState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledRunPin, ledRunState);
  }

  if (currentMillis - previousMillisRead >= intervalRead) {
    // save the last time you blinked the LED
    previousMillisRead = currentMillis;

    Serial.print("Initializing SD card...");

    if (!SD.begin(4)) {
      Serial.println("initialization failed!");
      return;
    }
    Serial.println("initialization done.");

    digitalWrite(ledReadPin, HIGH);

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);


    DateTime now = rtc.now();

    String mo = String(now.month(), DEC);
    if ( now.month() < 10 ) {
      mo = "0";
      mo.concat(now.month());
    }

    String da = String(now.day(), DEC);
    if ( now.day() < 10 ) {
      da = "0";
      da.concat(now.day());
    }

    String ho = String(now.hour(), DEC);
    if ( now.hour() < 10 ) {
      ho = "0";
      ho.concat(now.hour());
    }

    String mi = String(now.minute(), DEC);
    if ( now.minute() < 10 ) {
      mi = "0";
      mi.concat(now.minute());
    }

    String se = String(now.second(), DEC);
    if ( now.second() < 10 ) {
      se = "0";
      se.concat(now.second());
    }

    timeNow = "";
    timeNow.concat(now.year());
    timeNow.concat("-");
    timeNow.concat(mo);
    timeNow.concat("-");
    timeNow.concat(da);
    timeNow.concat(" ");
    timeNow.concat(ho);
    timeNow.concat(":");
    timeNow.concat(mi);
    timeNow.concat(":");
    timeNow.concat(se);

    readLine = "";
    readLine.concat(timeNow);
    readLine.concat(",");
    readLine.concat(h);
    readLine.concat(",");
    readLine.concat(t);

    Serial.println(readLine);

    String temp = "";
    temp.concat(now.year());
    temp.concat("-");
    temp.concat(mo);
    temp.concat(".csv");

    char fileName[temp.length() + 1];
    temp.toCharArray(fileName, sizeof(fileName));

    myFile = SD.open(fileName, FILE_WRITE);

    // if the file opened okay, write to it:
    if (myFile) {
      Serial.print("Writing to " + temp + "...");
      myFile.println(readLine);
      // close the file:
      myFile.close();
      Serial.println("done.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening " + temp);
    }

    SD.end();
  }
}
