#pragma once
#include "configuration.h"
#include "zifra_time.h"

#ifndef _Zifra_
#define _Zifra_

class Zifra {
public:
  ZifraConfig conf;
  CurrentTime time;
  Zifra(WiFiUDP & ntpUDP): conf(), time(ntpUDP, conf){
  }
private:
  

};


#endif
