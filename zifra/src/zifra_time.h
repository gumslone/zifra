#pragma once
#include <ds3231.h>
#include <NTPClient.h>
#include "dst_logic.h"
#ifndef ZIFRA_TIME_H
#define ZIFRA_TIME_H

// The optional DS3231 backup clock. It keeps UTC: the configured offset and
// the summer hour are applied on read, so an offline clock still switches
// correctly and a changed time zone needs no RTC rewrite.
class RtcTime {
  public:
    explicit RtcTime(ZifraConfig &conf): m_conf(conf) {
      if (!m_conf.wifiActive && m_conf.DS3231_active) {
        DS3231_init(DS3231_CONTROL_INTCN);
      }
    }

    time_t readUtc() {
      struct ts t {};
      DS3231_get(&t);
      return utcEpochFrom(t.year, t.mon, t.mday, t.hour, t.min, t.sec);
    }

    void writeUtc(time_t utc) {
      const struct tm *g = gmtime(&utc);
      struct ts t {};
      t.sec = g->tm_sec;
      t.min = g->tm_min;
      t.hour = g->tm_hour;
      t.mday = g->tm_mday;
      t.mon = g->tm_mon + 1;
      t.year = g->tm_year + 1900;
      t.wday = g->tm_wday + 1; // the DS3231 counts weekdays 1-7
      DS3231_set(t);
      D_println(F("RTC set from NTP"));
    }

  private:
    ZifraConfig &m_conf;
};

// The clock's notion of "now": NTP (kept as UTC) while online, the RTC
// otherwise. Local time = UTC + configured offset + summer hour (dst_logic).
class CurrentTime {
  public:
    CurrentTime(WiFiUDP &ntpUDP, ZifraConfig &conf):
      m_conf(conf),
      m_ntp(ntpUDP, conf.ntpServer.c_str(), 0),
      m_rtc(conf) {
    }
    uint8_t getHoursIso() {
      return m_hours;
    }
    uint8_t getHours() {
      if (!m_conf.clock.iso) {
        if (m_hours > 12) {
          return m_hours - 12;
        } else if (m_hours == 0) {
          return 12;
        }
      }
      return m_hours;
    }
    uint8_t getMinutes() {
      return m_minutes;
    }
    uint8_t getSeconds() {
      return m_seconds;
    }
    uint8_t getMonthDay() {
      return m_day;
    }
    uint8_t getDay() {
      return m_weekDay;
    }
    uint8_t getWeekDay() {
      return getDay();
    }
    uint8_t getMonth() {
      return m_month;
    }
    uint16_t getYear() {
      return m_year;
    }
    int8_t getNum(uint8_t pos) {
      switch (pos) {
        case 0:
          return getHours() / 10;
        case 1:
          return getHours() % 10;
        case 2:
          return getMinutes() / 10;
        case 3:
          return getMinutes() % 10;
        case 4:
          return getSeconds() / 10;
        case 5:
          return getSeconds() % 10;
        default:
          return -1;
      }
    }

    void update() {
      if (m_conf.wifiActive) {
        m_ntp.update();
        const time_t utc = m_ntp.getEpochTime();
        if (utc < VALID_AFTER) {
          return; // not synced yet - keep whatever we showed last
        }
        setFromUtc(utc);
        syncRtc(utc, false);
      } else if (m_conf.DS3231_active) {
        setFromUtc(m_rtc.readUtc());
      }
    }

    // Writes the current NTP time to the RTC right away (config saves).
    void setRTC() {
      const time_t utc = m_ntp.getEpochTime();
      if (m_conf.wifiActive && utc >= VALID_AFTER) {
        syncRtc(utc, true);
      }
    }

    // Re-applies the config's time settings at runtime (no restart needed):
    // fresh NTP server, then an immediate resync. The offset and summer
    // time are read live from the config on every update.
    void applySettings() {
      m_ntp.setPoolServerName(m_conf.ntpServer.c_str());
      if (m_conf.wifiActive) {
        m_ntp.forceUpdate();
        update();
      }
    }

    void begin() {
      m_ntp.begin();
    }

  private:
    static constexpr time_t VALID_AFTER = 1600000000; // 2020-09-13: NTP has synced
    static constexpr unsigned long RTC_SYNC_MS = 24UL * 60UL * 60UL * 1000UL;

    int offsetAt(time_t utc) const {
      int offset = m_conf.utcOffsetInSeconds;
      if (m_conf.dstMode == 1 && euSummerTime(*gmtime(&utc))) {
        offset += 3600;
      } else if (m_conf.dstMode == 2) {
        const time_t standard = utc + m_conf.utcOffsetInSeconds;
        if (usSummerTime(*gmtime(&standard))) {
          offset += 3600;
        }
      }
      return offset;
    }

    void setFromUtc(time_t utc) {
      const time_t local = utc + offsetAt(utc);
      const struct tm *t = gmtime(&local);
      m_hours = t->tm_hour;
      m_minutes = t->tm_min;
      m_seconds = t->tm_sec;
      m_day = t->tm_mday;
      m_weekDay = t->tm_wday;
      m_month = t->tm_mon + 1;
      m_year = t->tm_year + 1900;
    }

    // Keeps the backup clock fresh: once right after the first NTP sync,
    // then daily (or immediately when forced).
    void syncRtc(time_t utc, bool force) {
      if (!m_conf.DS3231_active) {
        return;
      }
      if (!force && m_rtcSynced && millis() - m_lastRtcSync < RTC_SYNC_MS) {
        return;
      }
      m_rtc.writeUtc(utc);
      m_rtcSynced = true;
      m_lastRtcSync = millis();
    }

    uint8_t m_hours{0};
    uint8_t m_minutes{0};
    uint8_t m_seconds{0};
    uint8_t m_day{0};
    uint8_t m_weekDay{0};
    uint8_t m_month{0};
    uint16_t m_year{0};
    ZifraConfig &m_conf;
    NTPClient m_ntp;
    RtcTime m_rtc;
    bool m_rtcSynced{false};
    unsigned long m_lastRtcSync{0};
};

#endif
