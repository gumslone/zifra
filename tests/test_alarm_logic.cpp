#include "Arduino.h"
#include "alarm_logic.h"
#include "test_framework.h"

static AlarmProperties weekdayAlarm() {
  AlarmProperties a;
  a.time = "06:45";
  a.active = true;
  for (int d = 1; d <= 5; d++) a.weekdays[d] = 1; // Mon-Fri
  return a;
}

int main() {
  CASE("fires only when everything matches");
  AlarmProperties a = weekdayAlarm();
  CHECK(alarmShouldFire(a, "06:45", 1, 5));   // Monday 06:45:05

  CASE("inactive alarm never fires");
  a = weekdayAlarm();
  a.active = false;
  CHECK(!alarmShouldFire(a, "06:45", 1, 5));

  CASE("already-fired alarm does not re-fire");
  a = weekdayAlarm();
  a.fired = true;
  CHECK(!alarmShouldFire(a, "06:45", 1, 5));

  CASE("wrong time or weekday does not fire");
  a = weekdayAlarm();
  CHECK(!alarmShouldFire(a, "06:46", 1, 5)); // wrong minute
  CHECK(!alarmShouldFire(a, "06:45", 0, 5)); // Sunday not selected
  CHECK(!alarmShouldFire(a, "06:45", 6, 5)); // Saturday not selected

  CASE("only matches in the first seconds of the minute");
  a = weekdayAlarm();
  CHECK(alarmShouldFire(a, "06:45", 1, 0));
  CHECK(alarmShouldFire(a, "06:45", 1, ALARM_MATCH_WINDOW_SECONDS - 1));
  CHECK(!alarmShouldFire(a, "06:45", 1, ALARM_MATCH_WINDOW_SECONDS));
  CHECK(!alarmShouldFire(a, "06:45", 1, 59));

  CASE("default-constructed alarm is inert");
  AlarmProperties fresh;
  CHECK(!alarmShouldFire(fresh, "", 0, 0));

  CASE("ringing expires after the configured duration");
  const unsigned long MIN = 60000UL;
  CHECK(alarmStillRinging(0, 0, 10));            // just fired
  CHECK(alarmStillRinging(0, 10 * MIN, 10));     // exactly 10 min -> still on
  CHECK(alarmStillRinging(0, 10 * MIN + 59999, 10)); // 10:59.999 -> still on
  CHECK(!alarmStillRinging(0, 11 * MIN, 10));    // 11 min -> off

  CASE("expiry math works when millis has advanced far");
  const unsigned long late = 4000000000UL; // near the 32-bit wrap on-device
  CHECK(alarmStillRinging(late, late + 2 * MIN, 10));
  CHECK(!alarmStillRinging(late, late + 11 * MIN, 10));

  return SUMMARY();
}
