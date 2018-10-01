# arduino-thc
Arduino BLE Time Hacker Clock

A Time Hacker Clock is a device based on the Time Hacker Method (THM). THM combines simple productivity guidelines with devices that emit shapes or colors to quickly convey how much time remains in a given day or time block.


## Features

* Bluetooth BLE, constrollable via mobile phone
* Realtime clock
* Support for various NeoPixel lighting modules

## Parts

* [Adafruit Feather 32u4 Bluefruit LE](https://www.adafruit.com/product/2829)
* Adafruit NeoPixels. Works with RGB/RGBW strings and rings:
  * [NeoPixel 5050 RGB LED](https://www.adafruit.com/product/1655 )
  * [NeoPixel Stick - 8 x 5050 RGB LED](https://www.adafruit.com/product/1426)
  * [NeoPixel Ring - 12 x 5050 RGB LED](https://www.adafruit.com/product/1643)
  * [NeoPixel Ring - 12 x 5050 RGBW LED](https://www.adafruit.com/product/2853)
  * [NeoPixel Ring - 16 x 5050 RGB LED](https://www.adafruit.com/product/1463)
  * [NeoPixel FeatherWing - 4x8 RGB LED](https://www.adafruit.com/product/2945)
* [Adafruit DS3231 Precision RTC Breakout](https://www.adafruit.com/product/3013)
* Enclosures:
  * https://www.allelectronics.com/item/1551-fbk/pocket-size-project-box/1.html

## Software

In the Ardunio IDE

"From the Sketch menu, > Include Library > Manage Libraries...  In the text input box type in "NeoPixel". Look for "Adafruit NeoPixel by Adafruit" and select the latest version by clicking on the dropbox menu next to the Install button"
More on the Adafruit website: https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-installation

### Third party Bluetooth mobile apps

#### Android phone controller app
* https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal
* https://play.google.com/store/apps/details?id=com.adafruit.bluefruit.le.connect

#### iOS phone controller app
* https://learn.adafruit.com/bluefruit-le-connect-for-ios
* LightBlue Explorer: https://itunes.apple.com/us/app/lightblue/id557428110

