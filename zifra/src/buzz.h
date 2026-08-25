#pragma once

#ifndef _buzz_
#define _buzz_
#include "pitches.h"

constexpr uint8_t BUZZER_PIN = 4;

class Buzz {
public:

void startUpMelody()
{
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    // e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    const int noteDuration = 1000 / m_noteDurations[thisNote];
    tone(BUZZER_PIN, m_melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    const int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(BUZZER_PIN);
  }
}

void beep()
{
  tone(BUZZER_PIN, NOTE_C4, 1000 / 16);
  // stop the tone playing:
  delay(200);
  noTone(BUZZER_PIN);
}


private:

// notes in the melody:
int m_melody[8] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3,
                NOTE_G3, 0,       NOTE_B3, NOTE_C4
               };

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int m_noteDurations[8] = {4, 8, 8, 4, 4, 4, 4, 4};

};

#endif
