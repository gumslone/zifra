#pragma once
#include "zifra_types.h"
#ifndef ZIFRA_ALARM_LOGIC_H
#define ZIFRA_ALARM_LOGIC_H

// Pure decision logic for the alarms. No hardware access — host-testable
// (tests/test_alarm_logic.cpp).

// An alarm only matches within the first seconds of its minute, so a muted
// alarm cannot re-fire for the rest of that minute.
constexpr uint8_t ALARM_MATCH_WINDOW_SECONDS = 10;

// True when this alarm should start ringing now. `currentTime` is "HH:MM"
// (24h clock), `weekDay` 0 (Sunday) - 6 (Saturday).
inline bool alarmShouldFire(const AlarmProperties &alarm,
                            const String &currentTime, uint8_t weekDay,
                            uint8_t seconds) {
  return alarm.active && !alarm.fired && alarm.time == currentTime &&
         seconds < ALARM_MATCH_WINDOW_SECONDS && alarm.weekdays[weekDay] == 1;
}

// True while a fired alarm should keep ringing (it expires after
// `durationMinutes`). Unsigned arithmetic keeps this correct across the
// millis() rollover.
inline bool alarmStillRinging(unsigned long fireTime, unsigned long now,
                              unsigned long durationMinutes) {
  return ((now - fireTime) / 60000) <= durationMinutes;
}

#endif
