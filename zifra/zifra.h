#pragma once
#include "configuration.h"
#include "zifra_time.h"
#include "alarm.h"

#ifndef _Zifra_
#define _Zifra_
struct VolatileParameters{
  unsigned long clockDelay{0};
  uint8_t clockNum{0};
  unsigned long sleepShakeTime{0};
};
class Zifra {
public:
  ZifraConfig conf;
  CurrentTime time;
  ZifraAlarm alarm;
  VolatileParameters vol;
  
  Zifra(WiFiUDP & ntpUDP): conf(), time(ntpUDP, conf), alarm(conf, time){
  }
  bool sleep(){
      bool doSleep{false};
      if (conf.clock.sleepStart != "" && conf.clock.sleepFinish != "" &&
      conf.clock.sleep == true) {
        const uint8_t sleep_start_hour = conf.clock.sleepStart.substring(0, 2).toInt();
        const uint8_t sleep_start_minute = conf.clock.sleepStart.substring(3, 5).toInt();
    
        const uint8_t sleep_finish_hour = conf.clock.sleepFinish.substring(0, 2).toInt();
        const uint8_t sleep_finish_minute = conf.clock.sleepFinish.substring(3, 5).toInt();
        
        const uint16_t current_hours_with_minutes = time.getHours()  * 100 + time.getMinutes();
        const uint16_t start_hours_with_minutes = sleep_start_hour * 100 + sleep_start_minute;
        const uint16_t end_hours_with_minutes = sleep_finish_hour * 100 + sleep_finish_minute;
        doSleep = (sleep_start_hour > sleep_finish_hour &&
                   (current_hours_with_minutes >= start_hours_with_minutes || current_hours_with_minutes <= end_hours_with_minutes))
                  || (current_hours_with_minutes >= start_hours_with_minutes && current_hours_with_minutes <= end_hours_with_minutes);
      }
      return doSleep;
    }
private:
  

};


#endif
