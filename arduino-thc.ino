#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <RTClib.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#include "BluefruitConfig.h"

#define PIN 6
#define LEDS 8

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.
uint32_t currentColor = strip.Color(0, 0, 0);
uint32_t lastColor = currentColor;
uint32_t currentLEDCount = LEDS;
RTC_DS3231 rtc;

String startupError = String("");

void setup() {
  rtc.begin();
  if (rtc.lostPower()) {
    // startupError = String("RTC lost power, check/change batter and send set date/time command");
    // following line sets the RTC to the date & time this sketch was compiled
     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
//    rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  strip.begin();
  strip.show();

  setupBLE();
}

void reset() {
  lastColor = currentColor;
  currentColor = strip.Color(0, 0, 0);
  currentLEDCount = LEDS;
  setMatrix(currentColor, currentLEDCount);
}

void setupBLE(void) {
  if ( !ble.begin(VERBOSE_MODE) ) {
    // startupError = String("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?");
  }

  ble.factoryReset();

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  ble.verbose(false);  // debug info is a little annoying after this point!

  delay(2000);
  ble.print("AT+GAPDEVNAME=");
  ble.println("Time Hacker Clock");

  /* Wait for connection */
  while (! ble.isConnected()) {
    delay(500);
  }

  delay(2000);
  ble.print("AT+BLEUARTTX=");
  ble.println("Time Hacker Clock ver 0.2.0\n");
  ble.waitForOK();

//   if (startupError.length() > 0) {
//     ble.print("AT+BLEUARTTX=");
//     ble.println(startupError);
//     ble.waitForOK();
//   }

  delay(1000);
  reset();
}

void loop() {
  DateTime now = rtc.now();

  // Check for incoming characters from Bluefruit
  ble.println("AT+BLEUARTRX");
  ble.readline();

  String command = String(ble.buffer);

  if (strcmp(ble.buffer, "reset") == 0) {
    reset();
    ble.print("AT+BLEUARTTX=");
    ble.println("color reset\n");
    ble.waitForOK();
  }

  if (command.startsWith("heat")) {
    String value = command.substring(4);
    value.trim();
    ble.print("AT+BLEUARTTX=");
    ble.println(value);
    ble.waitForOK();
    heat(value.toInt());
  }

  if (command.startsWith("time")) {
    ble.print("AT+BLEUARTTX=");
//    ble.println(now.unixtime());
    ble.print(now.hour());
    ble.print(now.minute());
    ble.print(now.second());
    ble.println(" ");
    ble.waitForOK();
  }

  if (command.startsWith("count")) {
    String value = command.substring(5);
    value.trim();
    ble.print("AT+BLEUARTTX=");
    ble.println(value);
    ble.waitForOK();
    reset();
    setLEDCount(value.toInt());
    currentColor = lastColor;
  }

  setMatrix(currentColor, currentLEDCount);
  delay(1000);
}

void setMatrix(uint32_t color, uint32_t count) {
  for (int i = 0; i < count; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void heat(uint32_t percent) {
  if (percent == 0) {
    currentColor = strip.Color(0, 0, 0);
    return;
  }
  uint32_t  r = percent < 50 ? 255 : floor(255 - (percent * 2 - 100) * 255 / 100);
  uint32_t  g = percent > 50 ? 255 : floor((percent * 2) * 255 / 100);
  currentColor = strip.Color(r, g, 0);
}

void setLEDCount(uint32_t count) {
  currentLEDCount = count;
}


