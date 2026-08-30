#pragma once
#include <Arduino.h>
#ifndef ZIFRA_MELODIES_H
#define ZIFRA_MELODIES_H

// Alarm sounds, played through the piezo buzzer as looping note sequences.
// freq 0 is a rest. Keep every note >= 150 ms: the alarm player advances on
// the 100 ms display tick, so shorter notes would smear.

struct MelodyNote {
  uint16_t freq;
  uint16_t ms;
};

struct Melody {
  const MelodyNote *notes;
  uint8_t count;
};

// 0: the original two-tone alarm
const MelodyNote MELODY_CLASSIC[] = {{500, 180}, {780, 180}};
// 1: two short beeps, then a pause
const MelodyNote MELODY_DOUBLE_BEEP[] = {{880, 150}, {0, 150}, {880, 150}, {0, 600}};
// 2: rising and falling sweep
const MelodyNote MELODY_SIREN[] = {{400, 150}, {520, 150}, {660, 150},
                                   {780, 150}, {660, 150}, {520, 150}};
// 3: ascending chime (C5 E5 G5 C6)
const MelodyNote MELODY_CHIME[] = {{523, 200}, {659, 200}, {784, 200},
                                   {1047, 400}, {0, 420}};
// 4: falling third, like a cuckoo clock (E5 -> C5)
const MelodyNote MELODY_CUCKOO[] = {{659, 300}, {523, 400}, {0, 400},
                                    {659, 300}, {523, 400}, {0, 800}};
// 5: perky bugle-style call (G4 C5 E5 triad)
const MelodyNote MELODY_BUGLE[] = {{392, 150}, {523, 150}, {659, 300},
                                   {523, 150}, {392, 150}, {659, 300},
                                   {0, 300}};
// 6: one soft beep with a long pause
const MelodyNote MELODY_GENTLE[] = {{988, 150}, {0, 1000}};
// 7: SOS in morse (three short, three long, three short)
const MelodyNote MELODY_SOS[] = {
  {880, 150}, {0, 150}, {880, 150}, {0, 150}, {880, 150}, {0, 300},
  {880, 450}, {0, 150}, {880, 450}, {0, 150}, {880, 450}, {0, 300},
  {880, 150}, {0, 150}, {880, 150}, {0, 150}, {880, 150}, {0, 900}};

#define MELODY_ENTRY(notes) {notes, sizeof(notes) / sizeof((notes)[0])}
const Melody MELODIES[] = {
  MELODY_ENTRY(MELODY_CLASSIC),
  MELODY_ENTRY(MELODY_DOUBLE_BEEP),
  MELODY_ENTRY(MELODY_SIREN),
  MELODY_ENTRY(MELODY_CHIME),
  MELODY_ENTRY(MELODY_CUCKOO),
  MELODY_ENTRY(MELODY_BUGLE),
  MELODY_ENTRY(MELODY_GENTLE),
  MELODY_ENTRY(MELODY_SOS),
};
#undef MELODY_ENTRY
constexpr uint8_t MELODY_COUNT = sizeof(MELODIES) / sizeof(MELODIES[0]);

// Unknown indices fall back to the classic sound.
inline const Melody &melodyForIndex(int index) {
  if (index < 0 || index >= MELODY_COUNT) {
    return MELODIES[0];
  }
  return MELODIES[index];
}

// Pure: which note of the looping melody sounds at elapsed time t (ms).
// Host-testable (tests/test_melodies.cpp).
inline uint8_t melodyNoteIndexAt(const Melody &melody, unsigned long t) {
  unsigned long total = 0;
  for (uint8_t i = 0; i < melody.count; i++) {
    total += melody.notes[i].ms;
  }
  if (total == 0) {
    return 0; // empty melody: nothing to play
  }
  t %= total;
  uint8_t i = 0;
  while (t >= melody.notes[i].ms) {
    t -= melody.notes[i].ms;
    i++;
  }
  return i;
}

#endif
