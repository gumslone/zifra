#include "dst_logic.h"
#include "test_framework.h"

#include <time.h>

// Broken-down time for the given instant, with tm_wday filled in.
static struct tm at(int year, int month, int day, int hour, int minute = 0) {
  struct tm t {};
  t.tm_year = year - 1900;
  t.tm_mon = month - 1;
  t.tm_mday = day;
  t.tm_hour = hour;
  t.tm_min = minute;
  const time_t epoch = timegm(&t);
  struct tm out {};
  gmtime_r(&epoch, &out);
  return out;
}

int main() {
  CASE("EU: plain winter and summer months");
  CHECK(!euSummerTime(at(2026, 1, 15, 12)));
  CHECK(!euSummerTime(at(2026, 12, 31, 23)));
  CHECK(euSummerTime(at(2026, 7, 1, 0)));
  CHECK(euSummerTime(at(2026, 4, 1, 0)));

  CASE("EU: starts last Sunday of March 01:00 UTC (2026-03-29)");
  CHECK(!euSummerTime(at(2026, 3, 28, 23)));
  CHECK(!euSummerTime(at(2026, 3, 29, 0, 59)));
  CHECK(euSummerTime(at(2026, 3, 29, 1, 0)));
  CHECK(euSummerTime(at(2026, 3, 30, 0)));
  CHECK(euSummerTime(at(2026, 3, 31, 12)));

  CASE("EU: ends last Sunday of October 01:00 UTC (2026-10-25)");
  CHECK(euSummerTime(at(2026, 10, 24, 12)));
  CHECK(euSummerTime(at(2026, 10, 25, 0, 59)));
  CHECK(!euSummerTime(at(2026, 10, 25, 1, 0)));
  CHECK(!euSummerTime(at(2026, 10, 26, 0)));
  CHECK(!euSummerTime(at(2026, 10, 31, 12)));

  CASE("EU: other years (2025 switches on 03-30 and 10-26)");
  CHECK(!euSummerTime(at(2025, 3, 29, 12)));
  CHECK(!euSummerTime(at(2025, 3, 30, 0, 59)));
  CHECK(euSummerTime(at(2025, 3, 30, 1, 0)));
  CHECK(euSummerTime(at(2025, 10, 26, 0, 59)));
  CHECK(!euSummerTime(at(2025, 10, 26, 1, 0)));

  CASE("US: plain winter and summer months");
  CHECK(!usSummerTime(at(2026, 1, 15, 12)));
  CHECK(!usSummerTime(at(2026, 12, 31, 23)));
  CHECK(usSummerTime(at(2026, 6, 15, 12)));
  CHECK(usSummerTime(at(2026, 10, 31, 12)));

  CASE("US: starts second Sunday of March 02:00 (2026-03-08)");
  CHECK(!usSummerTime(at(2026, 3, 7, 23)));
  CHECK(!usSummerTime(at(2026, 3, 8, 1, 59)));
  CHECK(usSummerTime(at(2026, 3, 8, 2, 0)));
  CHECK(usSummerTime(at(2026, 3, 9, 0)));
  CHECK(usSummerTime(at(2026, 3, 15, 12)));

  CASE("US: ends first Sunday of November 02:00 (2026-11-01)");
  CHECK(usSummerTime(at(2026, 11, 1, 1, 59)));
  CHECK(!usSummerTime(at(2026, 11, 1, 2, 0)));
  CHECK(!usSummerTime(at(2026, 11, 2, 0)));
  CHECK(!usSummerTime(at(2026, 11, 30, 12)));

  CASE("US: other years (2025 switches on 03-09 and 11-02)");
  CHECK(!usSummerTime(at(2025, 3, 9, 1, 59)));
  CHECK(usSummerTime(at(2025, 3, 9, 2, 0)));
  CHECK(usSummerTime(at(2025, 11, 1, 12)));
  CHECK(usSummerTime(at(2025, 11, 2, 1, 59)));
  CHECK(!usSummerTime(at(2025, 11, 2, 2, 0)));

  return SUMMARY();
}
