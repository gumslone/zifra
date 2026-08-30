#include "Arduino.h"
#include "melodies.h"
#include "test_framework.h"

int main() {
  CASE("eight melodies, unknown indices fall back to classic");
  CHECK(MELODY_COUNT == 8);
  CHECK(&melodyForIndex(-1) == &MELODIES[0]);
  CHECK(&melodyForIndex(99) == &MELODIES[0]);
  CHECK(&melodyForIndex(2) == &MELODIES[2]);

  CASE("classic melody alternates its two tones and loops");
  const Melody &classic = MELODIES[0]; // {500,180},{780,180} -> 360ms loop
  CHECK(melodyNoteIndexAt(classic, 0) == 0);
  CHECK(melodyNoteIndexAt(classic, 179) == 0);
  CHECK(melodyNoteIndexAt(classic, 180) == 1);
  CHECK(melodyNoteIndexAt(classic, 359) == 1);
  CHECK(melodyNoteIndexAt(classic, 360) == 0);  // wrapped
  CHECK(melodyNoteIndexAt(classic, 360 * 1000 + 200) == 1); // still looping much later

  CASE("note boundaries walk the whole sequence in order");
  const Melody &chime = MELODIES[3]; // 200,200,200,400,420
  CHECK(melodyNoteIndexAt(chime, 0) == 0);
  CHECK(melodyNoteIndexAt(chime, 200) == 1);
  CHECK(melodyNoteIndexAt(chime, 400) == 2);
  CHECK(melodyNoteIndexAt(chime, 600) == 3);
  CHECK(melodyNoteIndexAt(chime, 1000) == 4);
  CHECK(melodyNoteIndexAt(chime, 1420) == 0); // full loop

  CASE("rests are real notes with freq 0");
  const Melody &beep = MELODIES[1];
  CHECK(beep.notes[1].freq == 0);
  CHECK(beep.notes[3].freq == 0);
  CHECK(beep.notes[melodyNoteIndexAt(beep, 1000)].freq == 0); // inside long rest

  CASE("player granularity: every note is at least 150 ms");
  for (uint8_t m = 0; m < MELODY_COUNT; m++) {
    for (uint8_t n = 0; n < MELODIES[m].count; n++) {
      CHECK(MELODIES[m].notes[n].ms >= 150);
    }
  }

  return SUMMARY();
}
