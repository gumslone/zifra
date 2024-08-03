#pragma once

#ifndef _Zifra_Alarm_
#define _Zifra_Alarm_

class ZifraAlarm {
  public:
    ZifraAlarm(ZifraConfig & conf, CurrentTime & time): m_conf(conf), m_time(time) {}
    void update()
    {
      activateAlarm();
      expire();
      playAlarm();
    }
    void mute() {
      m_conf.alarm1.fired = false;
      m_conf.alarm2.fired = false;
      m_conf.alarm3.fired = false;
      noTone(4);
    }

  private:
    ZifraConfig & m_conf;
    CurrentTime & m_time;
    bool m_alarmSound{false};

    void activateAlarm() {
      String currentTime =
        IntFormat(m_time.getHoursIso()) + ":" + IntFormat(m_time.getMinutes());
      uint8_t NumDayOfWeek = dayOfWeek(m_time.getYear(), m_time.getMonth(), m_time.getDay());
      if (m_conf.alarm1.active && !m_conf.alarm1.fired && m_conf.alarm1.time == currentTime && m_time.getSeconds() <= 5 &&
          m_conf.alarm1.weekdays[NumDayOfWeek] == 1) {
        m_conf.alarm1.fired = true;
        m_conf.alarm1.fireTime = millis();
        D_println(F("ALARM 1!"));
      } else if (m_conf.alarm2.active && !m_conf.alarm2.fired && m_conf.alarm2.time == currentTime && m_time.getSeconds() <= 5 &&
                 m_conf.alarm2.weekdays[NumDayOfWeek] == 1) {
        m_conf.alarm2.fired = true;
        m_conf.alarm2.fireTime = millis();
        D_println(F("ALARM 2!"));
      } else if (m_conf.alarm3.active && !m_conf.alarm3.fired && m_conf.alarm3.time == currentTime && m_time.getSeconds() <= 5 &&
                 m_conf.alarm3.weekdays[NumDayOfWeek] == 1) {
        m_conf.alarm3.fired = true;
        m_conf.alarm3.fireTime = millis();
        D_println(F("ALARM 3!"));
      }
    }
    bool expire()
    {
      if (m_conf.alarm1.fired) {
        m_conf.alarm1.fired = ((millis() - m_conf.alarm1.fireTime) / 60000) <= 10;
      } else if (m_conf.alarm2.fired) {
        m_conf.alarm2.fired = ((millis() - m_conf.alarm2.fireTime) / 60000) <= 10;
      } else if (m_conf.alarm3.fired) {
        m_conf.alarm3.fired = ((millis() - m_conf.alarm3.fireTime) / 60000) <= 10;
      }
    }
    void playAlarm() {
      if (m_conf.alarm1.fired || m_conf.alarm2.fired || m_conf.alarm3.fired) {
        if (m_alarmSound)
          tone(4, 780, 180);
        else
          tone(4, 500, 180);
        m_alarmSound = !m_alarmSound;
      }
    }
};
#endif
