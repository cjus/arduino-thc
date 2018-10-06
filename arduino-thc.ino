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

#define VERSION "0.6.0"
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
  Serial.begin(9600);
  delay(3000); // wait for console opening
//  Serial.print();
//  Serial.println();

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
  ble.println("Time Hacker Clock2");
  ble.waitForOK();

  /* Wait for connection */
  // while (!ble.isConnected()) {
  //   delay(500);
  // }
  // delay(2000);

  reset();
}

void loop() {
  processBLECommands();

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
  if (!ble.isConnected()) {
    return;
  }
  // Check for incoming characters from Bluefruit
  ble.println("AT+BLEUARTRX");
  ble.readline();

  String command = String(ble.buffer);
  if (!command.length()) {
    return;
  }

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

void handleReset() {
  reset();
  if (ble.isConnected()) {
    ble.print("AT+BLEUARTTX=");
    ble.println("reset\n");
    ble.waitForOK();
  }
}

void handleGetTime() {
  if (!ble.isConnected()) {
    return;
  }
  DateTime now = rtc.now();
  ble.print("AT+BLEUARTTX=");
  blePrintTime(now.hour(), now.minute(), now.second());
  ble.println(" ");
  ble.waitForOK();
}

void handleGetDate() {
  if (!ble.isConnected()) {
    return;
  }
  DateTime now = rtc.now();
  ble.print("AT+BLEUARTTX=");
  blePrintDate(now.year(), now.month(), now.day());
  ble.println(" ");
  ble.waitForOK();
}

void handleSetClock(String &command) {
  if (!ble.isConnected()) {
    return;
  }
  String value = command.substring(3);
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
  blePrintDate(now.year(), now.month(), now.day());
  ble.print(" ");
  blePrintTime(now.hour(), now.minute(), now.second());
  ble.println(" ");
  ble.waitForOK();
}

void blePrintDate(int16_t year, int8_t month, int8_t day) {
  ble.print(year);
  ble.print("/");
  if (month < 10) {
    ble.print("0");
  }
  ble.print(month);
  ble.print("/");
  if (day < 10) {
    ble.print("0");
  }
  ble.print(day);
}

void blePrintTime(int8_t hour, int8_t minute, int8_t second) {
  if (hour < 10) {
    ble.print("0");
  }
  ble.print(hour);
  ble.print(":");
  if (minute < 10) {
    ble.print("0");
  }
  ble.print(minute);
  ble.print(":");
  if (second < 10) {
    ble.print("0");
  }
  ble.print(second);
}

void handleSetRange(String &command) {
  if (!ble.isConnected()) {
    return;
  }

  String value = command.substring(3);
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
  setTimeVars();
}

void handleBrightness(String &command) {
  if (!ble.isConnected()) {
    return;
  }
  String value = command.substring(3);
  value.trim();
  brightness = value.toInt();
  Serial.println(brightness);  
  strip.setBrightness(brightness);
  strip.show();
}

void handleGetVersion() {
  if (!ble.isConnected()) {
    return;
  }
  ble.print("AT+BLEUARTTX=");
  ble.print("Time Hacker Clock ver ");
  ble.print(VERSION);
  ble.println("");
  ble.waitForOK();
}
