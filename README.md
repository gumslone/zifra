<a href="https://www.buymeacoffee.com/gumslone" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/default-orange.png" alt="Buy Me A Coffee" height="41" width="174"></a>

# ZIFRA
a single digit nixie tube clock <a href="https://www.youtube.com/watch?v=uE2CU6pZ4Vk" target="_blank">Video</a>

<a href="https://www.youtube.com/watch?v=uE2CU6pZ4Vk" target="_blank"><img src="https://i.ytimg.com/vi/p9QBpXv5QZc/maxresdefault.jpg" width="500"></a>

### Features:
- has a build in RTC (real time clock) chip, that allows accurate time readings when zifra is used in offline mode.
- uses Network Time Protocol (NTP) for very accurate time reading in online mode.
- has build in buzzer to setup an alarm (you can set up to 3 alarms) (shake zifra to turn off the alarm)
- set a sleep time to turn of the nixie tube at night (shake to show time for a duration of 3 minutes during sleep time)
- configuration web interface that can be accessed via web browser http://zifra.local
- update the clock to the latest firmware through the web interface

<img src="https://github.com/gumslone/zifra/blob/main/images/web.png?raw=true" width="500">
<img src="https://github.com/gumslone/zifra/blob/main/images/time.png?raw=true" width="500">
<img src="https://github.com/gumslone/zifra/blob/main/images/alarm.png?raw=true" width="500">
<img src="https://github.com/gumslone/zifra/blob/main/images/update.png?raw=true" width="500">

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
