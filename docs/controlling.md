# Controlling the clock

## Third party Bluetooth mobile apps

The Ardunio THC can be controlled using anyone of several third party mobile app.

#### Android phone controller app
* https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal
* https://play.google.com/store/apps/details?id=com.adafruit.bluefruit.le.connect

#### iOS phone controller app
* https://learn.adafruit.com/bluefruit-le-connect-for-ios
* LightBlue Explorer: https://itunes.apple.com/us/app/lightblue/id557428110

## Supported commands

THC commands consist of three letter names optionally preceeded by one or more parameters. Commands are submitted via one of the above mobile applications via the UART terminal.


| Command | Description |
|---|---|
| gtd |  Get the date set on the THC device. |
| gtm | Get the current time on the THC realtime clock. You must do this when powering up your THC for the first time and when it becomes time to replace the THC battery. A xxxx battery should last about a year. |
| sck | Set the current date and time. Date time is in `yyyymmdd hhmmss` format. Each field must be 2 digit and preceeded with a zero if necessary. The year is always a 4 digit field. Time must be in a 24 hour clock format. |
| srg | Set the duration of a time block by specifying a start hour and end hour. Each field must be 2 digit and perceeded with a zero if necessary. Time must be in a 24 hour clock format. For example to set a time block range from 5am to 8pm use: srg 05 20 |
| ver | Get the THC software version. |

---

[Return to main](../README.md) | [Build device](../docs/build.md) | [Software setup](../doc/software.md) | [Controlling the clock](../docs/controlling.md)
