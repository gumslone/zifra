<a href="https://www.buymeacoffee.com/gumslone" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/default-orange.png" alt="Buy Me A Coffee" height="41" width="174"></a>

# ZIFRA

**ZIFRA** is an open-source **single-tube nixie clock**: one Soviet-era **IN-12B nixie tube**
spells the time out digit by digit. Powered by an **ESP8266 (ESP8285)**, it syncs its time
over WiFi via **NTP**, keeps time offline with a **DS3231 RTC**, rings up to three alarms
with selectable melodies, and is configured from a **mobile-friendly web interface** —
no app needed. <a href="https://www.youtube.com/watch?v=uE2CU6pZ4Vk" target="_blank">Watch it in action</a>.

<a href="https://www.youtube.com/watch?v=uE2CU6pZ4Vk" target="_blank"><img src="https://i.ytimg.com/vi/p9QBpXv5QZc/maxresdefault.jpg" width="500"></a>

### Order

Want one without soldering? Check the
<a href="https://www.tindie.com/stores/gumslone/" target="_blank">gumslone store on Tindie</a> —
or build your own: the firmware, web interface and 3D-printable enclosure are all in this repo.

### Features:
- has a build in RTC (real time clock) chip, that allows accurate time readings when zifra is used in offline mode.
- uses Network Time Protocol (NTP) for very accurate time reading in online mode.
- has build in buzzer to setup an alarm (you can set up to 3 alarms) (shake zifra to turn off the alarm)
- set a sleep time to turn of the nixie tube at night (shake to show time for a duration of 3 minutes during sleep time)
- mobile-friendly configuration web interface that can be accessed via web browser http://zifra.local
- update the clock to the latest firmware through the web interface

### Web interface

<img src="https://github.com/gumslone/zifra/blob/main/images/web.png?raw=true" width="640">

<img src="https://github.com/gumslone/zifra/blob/main/images/time.png?raw=true" width="210"> <img src="https://github.com/gumslone/zifra/blob/main/images/alarm.png?raw=true" width="210"> <img src="https://github.com/gumslone/zifra/blob/main/images/system.png?raw=true" width="210">

### Firmware downloads

The repo ships only the stock board's release binary,
`firmware/zifra_esp8285.bin`. Every [release](https://github.com/gumslone/zifra/releases)
carries all four variants as downloadable assets:

- `zifra_esp8285_<version>.bin` — stock ZIFRA board (ESP8285)
- `zifra_esp8266_generic_<version>.bin` — generic 1M ESP8266 modules (ESP-01S, ESP-12, ...)
- `..._debug.bin` — same, with serial logging on 115200 baud

Building locally with `./build.sh all` puts all four under
`.build/<board>/<release|debug>/zifra.ino.bin`.

### Flashing over USB

For the first flash (or to recover a clock that can't reach the web updater),
ZIFRA bundles [BugZapper](https://github.com/gumslone/bugzapper) — a flasher with
a built-in serial monitor — as a git submodule under `tools/bugzapper`. esptool
is bundled, so it needs nothing but `python3`.

Clone with submodules (or pull it in afterwards):

```sh
git clone --recurse-submodules https://github.com/gumslone/zifra.git
# already cloned?
git submodule update --init
```

Then, from the repo root:

```sh
./flash.sh        # flash the bundled firmware/zifra_esp8285.bin (-f for another)
./flash.sh -l     # list detected serial ports
./bugzapper.sh    # GUI flasher + serial monitor (needs python3 with tkinter)
```
