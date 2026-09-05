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
- built-in RTC (real-time clock) chip for accurate time when ZIFRA is used offline.
- Network Time Protocol (NTP) for very accurate time in online mode.
- automatic summer/winter time switching (European and US daylight-saving rules).
- built-in buzzer with up to 3 alarms and selectable melodies (shake ZIFRA to mute a ringing alarm).
- night sleep window that turns the nixie tube off at night (shake to show the time for 3 minutes during sleep).
- mobile-friendly configuration web interface, served by the clock itself at
  http://zifra.local — no app, no cloud, works on any LAN.
- firmware updates over WiFi, straight from the web interface (optionally password-protected).

### Web interface

<img src="https://github.com/gumslone/zifra/blob/main/images/web.png?raw=true" width="640">

<img src="https://github.com/gumslone/zifra/blob/main/images/time.png?raw=true" width="210"> <img src="https://github.com/gumslone/zifra/blob/main/images/alarm.png?raw=true" width="210"> <img src="https://github.com/gumslone/zifra/blob/main/images/system.png?raw=true" width="210">

### First-time setup

1. **Power the clock** with a 5 V USB power supply. On the first boot the tube cycles
   once through all digits and the buzzer plays a short jingle.
2. **Connect it to your WiFi.** With no network configured, ZIFRA opens its own
   access point named `ZIFRA-XXXXXX`. Join it with your phone and the setup portal
   opens by itself (if not, browse to `192.168.4.1`). Pick your network, enter the
   password, and save — the clock reboots and joins your WiFi. The portal stays up
   for 3 minutes; after that the clock restarts and tries again.
3. **Open the web interface** at http://zifra.local (or the clock's IP address,
   which your router's device list shows). The dashboard displays the firmware
   version, WiFi quality, IP and a live demo of what the tube is doing.
4. **Set your time zone** on the *Clock* page: tap *Set offset from this phone*, pick
   your summer/winter rule (Europe/USA) — with automatic switching on, the stored
   offset is the winter (standard) time and the clock adds the summer hour itself.
   Saving applies instantly, no restart needed.

No WiFi where the clock lives? Configure it once anywhere, then it keeps time on its
DS3231 RTC even with WiFi off.

### Everyday use

- **Reading the time** — the single tube spells the time digit by digit:
  hours' tens, hours' ones, then (with the decimal dot lit) minutes' tens and
  minutes' ones, followed by a longer dark pause before the next round. With the
  leading zero turned off, `09:41` starts straight at the `9` and reads one digit
  sooner.
- **Alarms** — up to three, each with its own weekdays and melody (preview the
  melodies right in the web UI). A ringing alarm stops when muted or after the
  configurable auto-stop time. **Shake the clock or press its button to mute.**
- **Night sleep** — the tube goes dark during the configured window (it may span
  midnight, e.g. 22:00–06:30). Shake the clock to light the time up for 3 minutes.
  The dashboard shows when the clock is asleep.
- **WiFi on/off** — shake the clock continuously for about 10 seconds to toggle
  WiFi (it restarts and runs offline from the RTC).
- **Factory reset** — hold the button for 35 seconds: settings and WiFi
  credentials are wiped and the setup portal opens again.
- **Firmware updates** — pick a release `.bin` on the *System* page and flash it
  right there, over WiFi. Settings survive updates. Set an *update password* on
  the same page if other people share your network (user name `zifra`).

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

### Development

The firmware is a single Arduino sketch ([`zifra/zifra.ino`](zifra/zifra.ino)) with
its modules as headers under [`zifra/src/`](zifra/src) — pure logic (alarms, sleep
window, melodies, daylight-saving rules) is separated from hardware code so it can
be tested on the host:

```sh
./build.sh all     # arduino-cli builds: esp8285 + generic, release + debug
pio run            # or build with PlatformIO
./tests/run.sh     # native host tests, no board needed
./tests/tidy.sh    # clang-tidy static analysis
```

The web interface lives in [`web/v2/`](web/v2) — a dependency-free ES6 single-page
app in layered modules (`core` / `state` / `view` / `audio` / `app`). The clock
serves it itself: `tools/web/gzip_pages.py` (run by `build.sh`) bundles the JS and
CSS together with the pages in [`zifra/pages/`](zifra/pages) into gzipped PROGMEM
arrays in `Webinterface.h`, so no external hosting is involved. The same bundles
are also published at tehybug.com for firmware older than 2.5. The "Zifra Tube"
nixie webfont the UI uses is generated by
[`tools/font/build_font.py`](tools/font/build_font.py). CI builds every push and
publishes tagged releases from `zifra/src/version.h`.

### A short history of the nixie tube

Long before LEDs, numbers glowed in neon. The **nixie tube** is a cold-cathode
display from the 1950s: a glass envelope filled with neon holds a stack of ten
thin metal cathodes, each shaped like a digit, behind a wire-mesh anode. Apply
~170 V to one cathode and the gas around it ionizes — the digit is wrapped in the
soft orange glow of the neon discharge. Because the digits sit physically behind
one another, each one lights at its own depth inside the glass, which gives nixies
their unmistakable, slightly floating look.

Burroughs Corporation trademarked the name **Nixie** (from "Numeric Indicator
eXperimental No. 1") in 1955, and through the 1960s nixies were *the* readout of
serious machines: frequency counters, multimeters, desktop calculators, elevator
floor indicators, even mission-control consoles. By the mid-1970s cheaper,
lower-voltage vacuum-fluorescent displays and LEDs had displaced them in the
West — but factories in the Soviet Union kept producing their own **ИН (IN)
series** in enormous numbers well into the 1980s.

The tube in ZIFRA is one of those: the **IN-12B (ИН-12Б)**, a side-viewed nixie
with 18 mm digits and a decimal point on the left, made mainly by Reflector in
Saratov. Other IN-series tubes came from plants across the Soviet Union — the
Gazotron works in Rivne, Ukraine was among the biggest, producing the IN-8,
IN-14 and IN-16 that power many of today's nixie clocks. Unsocketed "new old stock" tubes still surface from old
warehouses, which is what keeps nixie clocks alive today — nobody mass-produces
nixies anymore, so every tube is decades old and finite. That scarcity is also why
ZIFRA uses a **single tube** that spells the time digit by digit instead of four
in a row: one tube to source, one tube to someday replace, and a clock that makes
a little ceremony out of every minute. The night-sleep window exists for the same
reason — a resting tube is a tube that lasts: powered sensibly, an IN-12B is good
for years of continuous glow, and cycling through all ten digits (as ZIFRA does at
startup) helps keep unused cathodes clean.
