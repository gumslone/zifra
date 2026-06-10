#pragma once
#include "configuration.h"
#include "zifra_time.h"
#include "buzz.h"
#include "alarm.h"
#include "zifra_clock_timer.h"
#include "pf595.h"
#ifndef _Zifra_
#define _Zifra_
struct VolatileParameters {
  unsigned long sleepShakeTime{0};
};
class Zifra {
public:
  ZifraConfig conf{};
  CurrentTime time; // has to be after conf
  ZifraAlarm alarm;
  VolatileParameters vol{};
  Buzz buzzer{};
  Zifra(WiFiUDP & ntpUDP): conf(), time(ntpUDP, conf), alarm(conf, time) {
  }

  void setUp()
  {
    conf.setCb([&]() { time.setRTC(); });
    time.setTimeOffset();
  }
  bool sleep() {
    if (!conf.clock.sleep || conf.clock.sleepStart == "" ||
        conf.clock.sleepFinish == "") {
      return false;
    }
    const uint16_t now = time.getHours() * 100 + time.getMinutes();
    const uint16_t start = toHoursWithMinutes(conf.clock.sleepStart);
    const uint16_t finish = toHoursWithMinutes(conf.clock.sleepFinish);
    if (start / 100 > finish / 100) { // sleep window wraps past midnight
      return now >= start || now <= finish;
    }
    return now >= start && now <= finish;
  }
  void off()
  {
    pf595.allOff();
  }
  void showTime() {
    alarm.update();
    const bool shakeWakeUp = vol.sleepShakeTime > 0 &&
                             (millis() - vol.sleepShakeTime) <= SHAKE_WAKE_UP_MS;
    if (!shakeWakeUp && sleep()) {
      off();
    } else {
      timeToNixie();
    }
  }
  void update() {
    time.update();
    clockTimer.reset();
  }
  void startUpNixie() {
    pf595.startUp();
  }
private:
  static constexpr unsigned long SHAKE_WAKE_UP_MS = 3UL * 60UL * 1000UL; // 3 minutes shake wakeup
  ClockTimer clockTimer{};
  PF595 pf595{};

  // "HH:MM" -> HHMM, e.g. "21:30" -> 2130
  static uint16_t toHoursWithMinutes(const String &hhmm) {
    return hhmm.substring(0, 2).toInt() * 100 + hhmm.substring(3, 5).toInt();
  }

  void timeToNixie()
  {
    const uint8_t val1 = time.getNum(0);
    const uint8_t val2 = time.getNum(1);
    const uint8_t val3 = time.getNum(2);
    const uint8_t val4 = time.getNum(3);

    if (val1 != 0 || conf.clock.leadingHourZero) {
      if (clockTimer.showDigit(0)) {
        pf595.showNumber(val1, false);
      }
      else if (clockTimer.showDigit(1)) {
        off();
      }
    }
    else
    {
      clockTimer.nextDigit();
    }

    if (clockTimer.showDigit(2)) {
      pf595.showNumber(val2, false);
    }
    else if (clockTimer.showDigit(3)) {
      off();
    }
    else if (clockTimer.showDigit(4)) {
      pf595.showNumber(val3, true);
    }
    else if (clockTimer.showDigit(5)) {
      off();
    }
    else if (clockTimer.showDigit(6)) {
      pf595.showNumber(val4, true);
    }
    else if (clockTimer.getCurrentDigit() >= 7) { // Don't call expired that resets the digit, just freeze clock at 7.
                                                  // A reset will be called in the read_time function.
      off();
    }
  }

};


#endif
