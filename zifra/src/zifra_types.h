#pragma once
#include <Arduino.h>
#ifndef ZIFRA_TYPES_H
#define ZIFRA_TYPES_H

// Plain data types shared across the firmware. Kept free of heavy includes
// (FS, ArduinoJson, WiFi) so the pure logic headers that read them can also
// compile on a desktop compiler for the host tests in tests/.

struct AlarmProperties {
  String time{};
  int weekdays[7] = {0, 0, 0, 0, 0, 0, 0};
  bool fired{false};
  unsigned long fireTime{0};
  bool active{false};
  int melody{0}; // index into MELODIES (melodies.h)
};

struct Clock {
  bool iso{true};
  bool leadingHourZero{true};
  bool sleep{false};
  String sleepStart{};
  String sleepFinish{};
};

#endif
