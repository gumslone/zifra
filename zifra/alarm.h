#pragma once
#include "alarm_logic.h"
#ifndef _Zifra_Alarm_
#define _Zifra_Alarm_

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
      noTone(BUZZER_PIN);
    }

  private:
    static constexpr unsigned long ALARM_DURATION_MINUTES = 10;
    ZifraConfig & m_conf;
    CurrentTime & m_time;
    bool m_alarmSound{false};

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
          alarm.fired =
            alarmStillRinging(alarm.fireTime, millis(), ALARM_DURATION_MINUTES);
          break;
        }
      }
    }
    bool anyFired() const {
      for (const auto &alarm : m_conf.alarms) {
        if (alarm.fired) {
          return true;
        }
      }
      return false;
    }
    void playAlarm() {
      if (anyFired()) {
        tone(BUZZER_PIN, m_alarmSound ? 780 : 500, 180);
        m_alarmSound = !m_alarmSound;
      }
    }
};
#endif
