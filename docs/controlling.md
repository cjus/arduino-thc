# Controlling the clock

The clock is controlled via a mobile app connected via Bluetooth.

Commands consist of three letter names optionally preceeded by one or more parameters.

| Command | Example | Description |
|---|---|---|
| brt | `brt 100` | Set brightness level. A level is from 10 to 100 percent. |
| gtd | `gtd` | Get the date set on the THC device. |
| gtm | `gtm` | Get the current time on the THC realtime clock. |
| rst | `rst` | Reset clock to default setting. Sets the time range to 5am - 8pm. |
| sck | `sck 20181005 103230` | Set the current date and time. You must do this when powering up your THC for the first time and when it becomes time to replace the THC battery. A xxxx battery should last about a year. Date time is in `yyyymmdd hhmmss` format. Each field must be 2 digit and preceeded with a zero if necessary. The year is always a 4 digit field. Time must be in a 24 hour clock format. |
| srg | `srg 05 20` | Set the range (duration) of a time block by specifying a start hour and end hour. Each field must be 2 digit and perceeded with a zero if necessary. Time must be in a 24 hour clock format. |
| ver | `ver` | Get the THC software version. |

## Third party Bluetooth mobile apps

Commands are submitted via one of the mobile apps listed below.

#### Android phone controller app
* https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal
* https://play.google.com/store/apps/details?id=com.adafruit.bluefruit.le.connect

#### iOS phone controller app
* https://learn.adafruit.com/bluefruit-le-connect-for-ios
* LightBlue Explorer: https://itunes.apple.com/us/app/lightblue/id557428110

---

[Return to main](../README.md) | [Build device](../docs/build.md) | [Software setup](../docs/software.md) | [Controlling the clock](../docs/controlling.md)
