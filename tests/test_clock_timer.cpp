#include "Arduino.h"
#include "zifra_clock_timer.h"
#include "test_framework.h"

// The digit-slot schedule from zifra_clock_timer.h:
// slot:  0    1    2    3    4    5    6    7
// ms:   800  200  800  700  800  200  800  1600
// Even slots show a digit, odd slots are dark pauses, slot 7 is the long
// dark gap before the cycle repeats.

int main() {
  CASE("fresh timer shows slot 0");
  g_millis = 0;
  ClockTimer t;
  CHECK(t.getCurrentDigit() == 0);
  CHECK(t.showDigit(0));
  CHECK(!t.showDigit(1));

  CASE("slot 0 holds until its 800ms elapse");
  g_millis = 799;
  CHECK(t.showDigit(0));
  g_millis = 800; // expired: advances to slot 1
  CHECK(!t.showDigit(0));
  CHECK(t.showDigit(1));

  CASE("the full slot schedule advances in order");
  const uint16_t durations[8] = {800, 200, 800, 700, 800, 200, 800, 1600};
  // We are at slot 1, whose interval started at g_millis == 800.
  unsigned long slotStart = 800;
  for (uint8_t slot = 1; slot < 7; slot++) {
    CHECK(t.getCurrentDigit() == slot);
    g_millis = slotStart + durations[slot] - 1;
    CHECK(t.showDigit(slot)); // still inside the slot
    g_millis = slotStart + durations[slot];
    CHECK(!t.showDigit(slot)); // expired -> advanced
    slotStart = g_millis;
  }
  CHECK(t.getCurrentDigit() == 7);

  CASE("after slot 7 expires the cycle wraps to slot 0");
  g_millis = slotStart + durations[7];
  CHECK(!t.showDigit(7)); // expired: advances to 8...
  CHECK(t.getCurrentDigit() == 0); // ...which reads as 0 again

  CASE("reset returns to slot 0 and restarts the interval");
  g_millis += 12345;
  t.nextDigit();
  t.reset();
  CHECK(t.getCurrentDigit() == 0);
  CHECK(t.showDigit(0)); // fresh interval from the reset time
  g_millis += 799;
  CHECK(t.showDigit(0));

  CASE("firmware freeze convention: slot 7 stays dark if expired() is not called");
  t.reset();
  // timeToNixie() checks getCurrentDigit() >= 7 WITHOUT calling showDigit(7)
  // so the timer never advances past the long dark slot until update() resets
  // it. getCurrentDigit() alone must not advance the slot.
  for (int i = 0; i < 7; i++) t.nextDigit();
  g_millis += 100000; // way past every interval
  CHECK(t.getCurrentDigit() == 7);
  CHECK(t.getCurrentDigit() == 7); // still 7: reading does not advance

  return SUMMARY();
}
