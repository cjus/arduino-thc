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

#define VERSION "0.8.0"
#define PIN 6
#define RGBW false // is 4 LED Neopixel? RGB+White
#define LEDS 1 // total number of LEDS in strip or ring

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, PIN, ((RGBW) ? NEO_GRBW : NEO_GRB) + NEO_KHZ800);
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

uint32_t currentColor = strip.Color(0, 0, 0);
uint32_t lastColor = currentColor;
uint8_t brightness = 100;
uint8_t startHourSet = 5; // 5am
uint8_t endHourSet = 20; // 8pm
uint32_t startTime;
uint32_t endTime;

uint8_t currentLEDCount = LEDS;
uint8_t lastPercentRemaining = 0;

RTC_DS3231 rtc;

void setup() {
  // Serial.begin(9600);
  // delay(3000); // wait for console opening
  // Serial.print();
  // Serial.println();

  rtc.begin();
  setTimeVars();

  strip.setBrightness(brightness);
  strip.begin();
  setupBLE();
  reset();
}

void setTimeVars() {
  DateTime now = rtc.now();
  startTime = DateTime(now.year(), now.month(), now.day(), startHourSet, 0, 0).unixtime();
  endTime = DateTime(now.year(), now.month(), now.day(), endHourSet, 0, 0).unixtime();
}

void reset() {
  lastColor = currentColor;
  currentColor = strip.Color(0, 0, 0);
  currentLEDCount = LEDS;
  brightness = 100;
  startHourSet = 5; // 5am
  endHourSet = 20; // 8pm

  setMatrix(currentColor, currentLEDCount);
  setTimeVars();
}

void setupBLE(void) {
  if (!ble.begin(VERBOSE_MODE)) {
    // startupError = String("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?");
  }

  ble.factoryReset();
  ble.echo(false); // Disable command echo from Bluefruit
  ble.verbose(false);  // debug info is a little annoying after this point!
  delay(2000);

  ble.print("AT+GAPDEVNAME=");
  ble.println("Time Hacker Clock");
  ble.waitForOK();
  reset();
}

void loop() {
  if (ble.isConnected()) {
    processBLECommands();
  }

  DateTime now = rtc.now();
  float percentFactor = 100.0;
  uint32_t curTime = now.unixtime();

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

  if (strcmp(ble.buffer, "OK") == 0) {
    return;
  }

  String command = String(ble.buffer);
  if (strcmp(ble.buffer, "rst") == 0) {
    handleReset();
    return;
  }

  if (command.startsWith("gtm")) {
    handleGetTime();
    return;
  }

  if (command.startsWith("gdt")) {
    handleGetDate();
    return;
  }

  if (command.startsWith("grg")) {
    handleGetRange();
    return;
  }

  if (command.startsWith("sck")) {
    handleSetClock(command);
    return;
  }

  if (command.startsWith("srg")) {
    handleSetRange(command);
    return;
  }

  if (command.startsWith("brt")) {
    handleBrightness(command);
    return;
  }

  if (command.startsWith("ver")) {
    handleGetVersion();
    return;
  }

  String s = "Error, unknown command: ";
  s += command;
  blePrint(s);
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

void handleReset() {
  reset();
  String s = "reset";
  blePrint(s);
}

void handleGetTime() {
  DateTime now = rtc.now();
  blePrintTime(now.hour(), now.minute(), now.second());
}

void handleGetDate() {
  DateTime now = rtc.now();
  blePrintDate(now.year(), now.month(), now.day());
}

void handleSetClock(String &command) {
  String value = command.substring(3);
  value.trim();

  if (value.length() != 15) {
    // 20180929 183429
    String s = "Invalid date format use: yyyymmdd hhmmss";
    blePrint(s);
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

  String s = "Clock set to: ";
  blePrint(s);
  blePrintDate(now.year(), now.month(), now.day());
  blePrintTime(now.hour(), now.minute(), now.second());
}

void blePrintDate(int16_t year, int8_t month, int8_t day) {
  String s;
  s += year;
  s += "/";
  if (month < 10) {
    s += "0";
  }
  s += month;
  s += "/";
  if (day < 10) {
    s += "0";
  }
  s += day;
  blePrint(s);
}

void blePrintTime(int8_t hour, int8_t minute, int8_t second) {
  String s;
  if (hour < 10) {
    s += "0";
  }
  s += hour;
  s += ":";
  if (minute < 10) {
    s += "0";
  }
  s += minute;
  s += ":";
  if (second < 10) {
    s += "0";
  }
  s += second;
  blePrint(s);
}

void handleSetRange(String &command) {
  String value = command.substring(3);
  value.trim();

  if (value.length() != 5) {
    String s = "Invalid range format use: sh eh\\r\\nWhere sh = start hour and eh = hour on 24 hour clock. each must be two digits long";
    blePrint(s);
    return;
  }
  startHourSet = (value.substring(0, 2)).toInt();
  endHourSet = (value.substring(2)).toInt();
  setTimeVars();
  String s = "Range set: ";
  s += startHourSet;
  s += " ";
  s += endHourSet;
  blePrint(s);
}

void handleGetRange() {
  String s = "Time block range: ";
  s += startHourSet;
  s += " ";
  s += endHourSet;
  blePrint(s);
}

void handleBrightness(String &command) {
  String value = command.substring(3);
  value.trim();
  brightness = value.toInt();
  strip.setBrightness(brightness);
  strip.show();
  String s = "Brightness set to: ";
  s += brightness;
  blePrint(s);
}

void handleGetVersion() {
  String s = "Time Hacker Clock ver ";
  s += VERSION;
  blePrint(s);
}

void blePrint(String &s) {
  if (!ble.isConnected()) {
    return;
  }
  String ns = "AT+BLEUARTTX=";
  ns += s;
  ns += "\\r\\n";
  ble.println(ns);
  ble.waitForOK();
}

