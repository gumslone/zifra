#pragma once
#include "alarm_logic.h"
#include "melodies.h"
#ifndef ZIFRA_ALARM_H
#define ZIFRA_ALARM_H

class ZifraAlarm {
  public:
    ZifraAlarm(ZifraConfig & conf, CurrentTime & time): m_conf(conf), m_time(time) {}
    void update()
    {
      activateAlarms();
      expireAlarms();
      playAlarm();
    }
    void mute() {
      for (auto &alarm : m_conf.alarms) {
        alarm.fired = false;
      }
      m_ringing = false;
      noTone(BUZZER_PIN);
    }

  private:
    ZifraConfig & m_conf;
    CurrentTime & m_time;
    bool m_ringing{false};
    uint8_t m_lastNote{255};
    unsigned long m_ringStart{0};

    void activateAlarms() {
      const String currentTime =
        IntFormat(m_time.getHoursIso()) + ":" + IntFormat(m_time.getMinutes());
      const uint8_t weekDay = m_time.getWeekDay();
      for (uint8_t i = 0; i < ZifraConfig::ALARM_COUNT; i++) {
        AlarmProperties &alarm = m_conf.alarms[i];
        if (alarmShouldFire(alarm, currentTime, weekDay, m_time.getSeconds())) {
          alarm.fired = true;
          alarm.fireTime = millis();
          D_print(F("ALARM "));
          D_println(i + 1);
          break; // at most one alarm starts per update
        }
      }
    }
    void expireAlarms()
    {
      for (auto &alarm : m_conf.alarms) {
        if (alarm.fired) {
          alarm.fired = alarmStillRinging(
            alarm.fireTime, millis(),
            (unsigned long)m_conf.alarmTimeoutMinutes);
          break;
        }
      }
    }
    const AlarmProperties *firstRinging() const {
      for (const auto &alarm : m_conf.alarms) {
        if (alarm.fired) {
          return &alarm;
        }
      }
      return nullptr;
    }
    // Plays the ringing alarm's melody (melodies.h), advancing on the
    // display tick. When several alarms ring at once, the first one's
    // melody wins.
    void playAlarm() {
      const AlarmProperties *ringing = firstRinging();
      if (ringing == nullptr) {
        if (m_ringing) {
          m_ringing = false;
          noTone(BUZZER_PIN);
        }
        return;
      }
      if (!m_ringing) {
        m_ringing = true;
        m_ringStart = millis();
        m_lastNote = 255;
      }
      const Melody &melody = melodyForIndex(ringing->melody);
      const uint8_t note = melodyNoteIndexAt(melody, millis() - m_ringStart);
      if (note != m_lastNote) {
        m_lastNote = note;
        if (melody.notes[note].freq != 0) {
          tone(BUZZER_PIN, melody.notes[note].freq, melody.notes[note].ms);
        } else {
          noTone(BUZZER_PIN);
        }
      }
    }
};
#endif
