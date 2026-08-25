#include "Arduino.h"
#include "sleep_logic.h"
#include "test_framework.h"

int main() {
  CASE("toHoursWithMinutes parses HH:MM");
  CHECK(toHoursWithMinutes("21:30") == 2130);
  CHECK(toHoursWithMinutes("06:05") == 605);
  CHECK(toHoursWithMinutes("00:00") == 0);
  CHECK(toHoursWithMinutes("23:59") == 2359);

  CASE("same-day window, boundaries inclusive");
  CHECK(sleepWindowActive(1000, 900, 1700));
  CHECK(sleepWindowActive(900, 900, 1700));   // start boundary
  CHECK(sleepWindowActive(1700, 900, 1700));  // finish boundary
  CHECK(!sleepWindowActive(859, 900, 1700));
  CHECK(!sleepWindowActive(1701, 900, 1700));

  CASE("window wrapping past midnight");
  CHECK(sleepWindowActive(2300, 2200, 630));  // late evening
  CHECK(sleepWindowActive(30, 2200, 630));    // after midnight
  CHECK(sleepWindowActive(2200, 2200, 630));  // start boundary
  CHECK(sleepWindowActive(630, 2200, 630));   // finish boundary
  CHECK(!sleepWindowActive(631, 2200, 630));
  CHECK(!sleepWindowActive(1200, 2200, 630)); // midday

  CASE("shouldSleep requires enabled flag and a configured window");
  Clock clock;
  clock.sleepStart = "22:00";
  clock.sleepFinish = "06:30";
  clock.sleep = false;
  CHECK(!shouldSleep(clock, 2300)); // disabled
  clock.sleep = true;
  CHECK(shouldSleep(clock, 2300));
  CHECK(!shouldSleep(clock, 1200));
  clock.sleepStart = "";
  CHECK(!shouldSleep(clock, 2300)); // no start configured
  clock.sleepStart = "22:00";
  clock.sleepFinish = "";
  CHECK(!shouldSleep(clock, 2300)); // no finish configured

  CASE("regression: sleep must be decided on the 24h clock");
  // With the 12h display enabled the firmware once passed the 12h-converted
  // hour (22:30 -> 10:30) into the sleep decision, so a 22:00-06:30 window
  // never started in the evening. The caller now passes ISO (24h) hours;
  // these pin the distinction.
  Clock night;
  night.sleep = true;
  night.sleepStart = "22:00";
  night.sleepFinish = "06:30";
  CHECK(shouldSleep(night, 2230));   // 22:30, 24h clock -> asleep
  CHECK(!shouldSleep(night, 1030));  // the broken 12h-converted value -> awake

  return SUMMARY();
}
