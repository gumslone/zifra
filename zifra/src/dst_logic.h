#pragma once
#ifndef ZIFRA_DST_LOGIC_H
#define ZIFRA_DST_LOGIC_H

#include <time.h>

// Daylight-saving rules, evaluated on broken-down time. The configured
// utcOffsetInSeconds is always the standard (winter) offset; when a rule
// says summer time is active, one hour is added on top.
//
// dstMode config values: 0 = off, 1 = European rules, 2 = US rules.

// EU: summer time from the last Sunday of March 01:00 UTC until the last
// Sunday of October 01:00 UTC. Pass UTC time.
inline bool euSummerTime(const struct tm &utc) {
  const int month = utc.tm_mon + 1;
  if (month < 3 || month > 10) {
    return false;
  }
  if (month > 3 && month < 10) {
    return true;
  }
  // Date of the most recent Sunday (today when it is one); the switch
  // Sunday is the last of the month, i.e. the 25th or later.
  const int lastSunday = utc.tm_mday - utc.tm_wday;
  if (month == 3) {
    if (lastSunday < 25) {
      return false;
    }
    if (utc.tm_wday == 0) {
      return utc.tm_hour >= 1;
    }
    return true;
  }
  // October
  if (lastSunday < 25) {
    return true;
  }
  if (utc.tm_wday == 0) {
    return utc.tm_hour < 1;
  }
  return false;
}

// Seconds since 1970-01-01 00:00 UTC for a civil date and time given in
// UTC (days-from-civil algorithm, valid for 1970-2099). The RTC stores
// UTC as calendar fields; this turns them back into an epoch without
// depending on the C library's time zone state.
inline time_t utcEpochFrom(int year, int month, int day,
                           int hour, int minute, int second) {
  // count from March so leap days land at the end of the shifted year
  const int y = year - (month <= 2 ? 1 : 0);
  const int era = y / 400;
  const int yoe = y - era * 400;
  const int mp = (month + 9) % 12; // March = 0 ... February = 11
  const int doy = (153 * mp + 2) / 5 + day - 1;
  const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  const long days = (long)era * 146097 + doe - 719468;
  return (time_t)(days * 86400L + hour * 3600L + minute * 60L + second);
}

// US: summer time from the second Sunday of March 02:00 until the first
// Sunday of November 02:00, both local standard time. Pass standard time.
inline bool usSummerTime(const struct tm &standard) {
  const int month = standard.tm_mon + 1;
  if (month < 3 || month > 11) {
    return false;
  }
  if (month > 3 && month < 11) {
    return true;
  }
  const int lastSunday = standard.tm_mday - standard.tm_wday;
  if (month == 3) {
    // starts on the second Sunday, i.e. the 8th..14th
    if (lastSunday < 8) {
      return false;
    }
    if (standard.tm_wday == 0 && standard.tm_mday <= 14) {
      return standard.tm_hour >= 2;
    }
    return true;
  }
  // November: ends on the first Sunday, i.e. the 1st..7th
  if (lastSunday < 1) {
    return true;
  }
  if (standard.tm_wday == 0 && standard.tm_mday <= 7) {
    return standard.tm_hour < 2;
  }
  return false;
}

#endif
