#pragma once
#include "zifra_types.h"
#ifndef ZIFRA_SLEEP_LOGIC_H
#define ZIFRA_SLEEP_LOGIC_H

// Pure decision logic for the nixie night-sleep window. No hardware access —
// host-testable (tests/test_sleep_logic.cpp).

// "HH:MM" -> HHMM, e.g. "21:30" -> 2130
inline uint16_t toHoursWithMinutes(const String &hhmm) {
  return hhmm.substring(0, 2).toInt() * 100 + hhmm.substring(3, 5).toInt();
}

// True when `now` (HHMM) lies inside the window [start, finish], which may
// wrap past midnight (e.g. 2200 -> 0630).
inline bool sleepWindowActive(uint16_t now, uint16_t start, uint16_t finish) {
  if (start / 100 > finish / 100) { // sleep window wraps past midnight
    return now >= start || now <= finish;
  }
  return now >= start && now <= finish;
}

// Full decision: sleep enabled, window configured, and `now` (HHMM, 24h
// clock) inside it.
inline bool shouldSleep(const Clock &clock, uint16_t now) {
  if (!clock.sleep || clock.sleepStart == "" || clock.sleepFinish == "") {
    return false;
  }
  return sleepWindowActive(now, toHoursWithMinutes(clock.sleepStart),
                           toHoursWithMinutes(clock.sleepFinish));
}

#endif
