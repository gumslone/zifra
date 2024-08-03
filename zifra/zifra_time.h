#pragma once
#include <ds3231.h>
#include <NTPClient.h>
#ifndef _Zifra_Time_
#define _Zifra_Time_
class WebTime: public NTPClient {
  public:
    using NTPClient::NTPClient;
    uint16_t getYear() {
      unsigned long epochTime = getEpochTime();

      // Get a time structure
      struct tm *ptm = gmtime((time_t *)&epochTime);

      uint16_t currentYear = ptm->tm_year + 1900;
      return currentYear;
    }
    uint8_t getMonth()
    {
      unsigned long epochTime = getEpochTime();

      // Get a time structure
      struct tm *ptm = gmtime((time_t *)&epochTime);

      uint8_t currentMonth = ptm->tm_mon + 1;
      return currentMonth;
    }
    uint8_t getMonthDay()
    {
      unsigned long epochTime = getEpochTime();

      // Get a time structure
      struct tm *ptm = gmtime((time_t *)&epochTime);

      uint8_t monthDay = ptm->tm_mday;
      return monthDay;
    }
};
class RtcTime {
  public:
    RtcTime(ZifraConfig & conf): m_conf(conf) {
      if(!m_conf.wifiActive && m_conf.DS3231_active)
      {
        DS3231_init(DS3231_CONTROL_INTCN);
      }
    }
    uint8_t getHours() {
      return m_rtcTime.hour;
    }
    uint8_t  getMinutes() {
      return m_rtcTime.min;
    }
    uint8_t getSeconds() {
      return m_rtcTime.sec;
    }
    uint8_t getMonthDay() {
      return m_rtcTime.mday;
    }
    uint8_t getDay() {
      return m_rtcTime.wday;
    }
    uint8_t getMonth() {
      return m_rtcTime.mon + 1;
    }
    uint16_t getYear() {
      return 1900 + m_rtcTime.year;
    }
    void update()
    {
      if(m_conf.DS3231_active){DS3231_get(&m_rtcTime);}
    }
    void setRTC(WebTime & webTime)
    {
      webTime.update();
      m_rtcTime.hour = webTime.getHours();
      m_rtcTime.min = webTime.getMinutes();
      m_rtcTime.sec = webTime.getSeconds();
      m_rtcTime.mday = webTime.getMonthDay();
      m_rtcTime.wday = webTime.getDay();
      m_rtcTime.mon = webTime.getMonth() -1;
      m_rtcTime.year = webTime.getYear()-1900;
      DS3231_set(m_rtcTime);
      D_println(F("Saved RTC time"));
    }
  private:
    struct ts m_rtcTime {}; // for ds2320 rtc time
    ZifraConfig & m_conf;
    
};
class CurrentTime {
  public:
    CurrentTime(WiFiUDP & ntpUDP, ZifraConfig & conf): 
      m_webTime(ntpUDP, conf.ntpServer.c_str(), conf.utcOffsetInSeconds), 
      m_rtc(conf), 
      m_conf(conf) {  
     }
    uint8_t getHoursIso()
    {
      return m_hours;
    }
    uint8_t getHours()
    {
      if (!m_conf.clock.iso){
        if(m_hours > 12) {  
          return m_hours - 12;
        } else if (m_hours == 0) {
          return 12;
        }
      }
      return m_hours;
    }
    uint8_t getMinutes()
    {
      return m_minutes;
    }
    uint8_t getSeconds()
    {
      return m_seconds;
    }
    uint8_t getMonthDay()
    {
      return m_day;
    }
    uint8_t getDay()
    {
      return m_weekDay;
    }
    uint8_t getWeekDay() {
      return getDay();
    }
    uint8_t getMonth()
    {
      return m_month;
    }
    uint16_t getYear()
    {
      return m_year;
    }
    int8_t getNum(uint8_t pos)
    {
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
    void update()
    {
      if (m_conf.wifiActive) {
        m_webTime.update();
        m_hours = m_webTime.getHours();
        m_minutes = m_webTime.getMinutes();
        m_seconds = m_webTime.getSeconds();
        m_day = m_webTime.getMonthDay();
        m_weekDay = m_webTime.getDay();
        m_month = m_webTime.getMonth();
        m_year = m_webTime.getYear();
      } else if (m_conf.DS3231_active) {
        m_rtc.update();
        m_hours = m_rtc.getHours();
        m_minutes = m_rtc.getMinutes();
        m_seconds = m_rtc.getSeconds();
        m_day = m_rtc.getMonthDay();
        m_weekDay = m_rtc.getDay();
        m_month = m_rtc.getMonth();
        m_year = m_rtc.getYear();
      }
    }
    void printTime(bool updateTime = false)
    {
      if (updateTime)
      {
        // To do, move config to a class
        //update();
      }
      D_print(" Date : ");
      D_print(m_day);
      D_print("/");
      D_print(m_month);
      D_print("/");
      D_print(m_year);
      D_print("\t Hour : ");
      D_print(m_hours);
      D_print(":");
      D_print(m_minutes);
      D_print(".");
      D_println(m_seconds);
    }
    void setRTC()
    {
      m_rtc.setRTC(m_webTime);
    }
    void setTimeOffset()
    {
      m_webTime.setTimeOffset(m_conf.utcOffsetInSeconds);
    }
    void begin() 
    {
      m_webTime.begin();
    }
    
  private:
    uint8_t m_hours{0};
    uint8_t m_minutes{0};
    uint8_t m_seconds{0};
    uint8_t m_day{0};
    uint8_t m_weekDay{0};
    uint8_t m_month{0};;
    uint16_t m_year{0};
    ZifraConfig & m_conf;
    WebTime m_webTime;
    RtcTime m_rtc;
    
};

#endif
