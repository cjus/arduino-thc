#include <Arduino.h>
#include <string.h>
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
#define LEDS 24

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

uint32_t currentColor = strip.Color(0, 0, 0);
uint32_t lastColor = currentColor;
uint8_t startHourSet = 5; // 5am
uint8_t endHourSet = 20; // 8pm
uint8_t currentLEDCount = LEDS;
uint8_t lastPercentRemaining = 0;

RTC_DS3231 rtc;

void setup() {
//  Serial.begin(9600);
//  delay(3000); // wait for console opening
//  Serial.print();
//  Serial.println();
  rtc.begin();
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
  if (!ble.begin(VERBOSE_MODE)) {
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
  while (!ble.isConnected()) {
    delay(500);
  }

  delay(2000);
  ble.print("AT+BLEUARTTX=");
  ble.println("Time Hacker Clock ver 0.3.0");
  ble.println("");
  ble.waitForOK();

  delay(1000);
  reset();
}

void loop() {
  processBLECommands();

  DateTime now = rtc.now();
  float percentFactor = 100.0;
  uint32_t curTime = now.unixtime();
  uint32_t startTime = DateTime(now.year(), now.month(), now.day(), startHourSet, 0, 0).unixtime();
  uint32_t endTime = DateTime(now.year(), now.month(), now.day(), endHourSet, 0, 0).unixtime();

  if (curTime < startTime || curTime > endTime) {
    reset();
    return;
  }

  float range = endTime - startTime;
  float distance = (endTime - curTime);
  float percentRemaining = ((float)(distance / range) * percentFactor);

  currentLEDCount = (int8_t)(LEDS * (percentRemaining * 0.01));
  if (currentLEDCount == 0) {
    currentLEDCount = 1;
  }
  percentRemaining = (int32_t)percentRemaining;

  if (percentRemaining != lastPercentRemaining) {
    lastPercentRemaining = percentRemaining;
    heat(percentRemaining);
    setMatrix(currentColor, currentLEDCount);
  }
  delay(1000);
}

void processBLECommands() {
  // Check for incoming characters from Bluefruit
  ble.println("AT+BLEUARTRX");
  ble.readline();

  String command = String(ble.buffer);
  if (!command.length()) {
    return;
  }

  if (strcmp(ble.buffer, "rt") == 0) {
    reset();
    ble.print("AT+BLEUARTTX=");
    ble.println("reset\n");
    ble.waitForOK();
  }

  else if (command.startsWith("ti")) {
    DateTime now = rtc.now();
    ble.print("AT+BLEUARTTX=");
    ble.print(now.hour());
    ble.print(now.minute());
    ble.print(now.second());
    ble.println(" ");
    ble.waitForOK();
  }

  else if (command.startsWith("dt")) {
    DateTime now = rtc.now();
    ble.print("AT+BLEUARTTX=");
    ble.print(now.year());
    ble.print(now.month());
    ble.print(now.day());
    ble.println(" ");
    ble.waitForOK();
  }

  else if (command.startsWith("sc")) {
    String value = command.substring(2);
    value.trim();

    if (value.length() != 15) {
      // 20180929 183429
      ble.print("AT+BLEUARTTX=");
      ble.println("Invalid date format use: yyyymmdd hhmmss");
      ble.println(" ");
      ble.waitForOK();
      return;
    }

    int16_t year = (value.substring(0, 4)).toInt();
    int8_t month = (value.substring(4, 6)).toInt();
    int8_t day = (value.substring(6, 8)).toInt();
    int8_t hour = (value.substring(9, 11)).toInt();
    int8_t minute = (value.substring(11, 13)).toInt();
    int8_t second = (value.substring(13)).toInt();

    rtc.adjust(DateTime(year, month, day, hour, minute, second));
    DateTime now = rtc.now();

    ble.print("AT+BLEUARTTX=");
    ble.print("clock set to: ");
    ble.print(now.year());
    ble.print(now.month());
    ble.print(now.day());
    ble.print(" ");
    ble.print(now.hour());
    ble.print(now.minute());
    ble.print(now.second());
    ble.println(" ");
    ble.waitForOK();
  }

  else if (command.startsWith("sb")) {
    String value = command.substring(2);
    value.trim();

    if (value.length() != 5) {
      // 05 18
      ble.print("AT+BLEUARTTX=");
      ble.print("Invalid range format use: sh eh");
      ble.println(" ");
      ble.print("Where sh = start hour and eh = hour on 24 hour clock. each must be two digits long");
      ble.println(" ");
      ble.waitForOK();
      return;
    }
    startHourSet = (value.substring(0, 2)).toInt();
    endHourSet = (value.substring(2)).toInt();
  }
}

void setMatrix(uint32_t color, uint8_t count) {
  uint32_t black = strip.Color(0, 0, 0);
  for (int i = 0; i < LEDS; i++) {
    strip.setPixelColor(i, (i < count) ? color : black);
  }
  strip.show();
}

void heat(uint8_t percent) {
  if (percent == 0) {
    currentColor = strip.Color(0, 0, 0);
    return;
  }
  uint8_t  r = percent < 50 ? 255 : floor(255 - (percent * 2 - 100) * 255 / 100);
  uint8_t  g = percent > 50 ? 255 : floor((percent * 2) * 255 / 100);
  currentColor = strip.Color(r, g, 0);
}
