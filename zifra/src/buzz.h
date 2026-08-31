#pragma once
#include "pitches.h"
#ifndef ZIFRA_BUZZ_H
#define ZIFRA_BUZZ_H

constexpr uint8_t BUZZER_PIN = 4;

// The piezo buzzer: the power-on jingle and the button-press beep.
// (Alarm melodies live in melodies.h and are played by ZifraAlarm.)
class Buzz {
  public:
    void startUpMelody() {
      static const int notes[8] = {
        NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
      };
      // note durations: 4 = quarter note, 8 = eighth note, ...
      static const int durations[8] = {4, 8, 8, 4, 4, 4, 4, 4};
      for (uint8_t i = 0; i < 8; i++) {
        const int noteDuration = 1000 / durations[i];
        tone(BUZZER_PIN, notes[i], noteDuration);
        // the note's duration + 30% keeps the notes distinguishable
        delay(noteDuration * 13 / 10);
        noTone(BUZZER_PIN);
      }
    }

    void beep() {
      tone(BUZZER_PIN, NOTE_C4, 1000 / 16);
      delay(200);
      noTone(BUZZER_PIN);
    }
};

#endif
